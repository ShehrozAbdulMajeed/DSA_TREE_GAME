#include "Act2Screen.h"
#include "LevelFile.h"
#include "TextUtil.h"

static const float WIN_W = 1280.0f, WIN_H = 720.0f;

Act2Screen::Act2Screen(const sf::Font& font, Progress& progress)
	: prog(progress), view(font), text(font, "", 16) {
	LevelDef all[24];
	int n = loadLevelDefs(all, 24);
	defCount = 0;
	for (int i = 0; i < n && defCount < 16; i++) if (all[i].act == 2) defs[defCount++] = all[i];
	mode = SELECT; sel = 0; cutPage = 0; mx = my = 0; deliveryTimer = 0; ended = false; exitFlag = false; dirty = false;
}

bool Act2Screen::unlocked(int i) const { return i == 0 || prog.stars[2][i - 1] > 0; }
bool Act2Screen::inside(float px, float py, float x, float y, float w, float h) { return px >= x && px <= x + w && py >= y && py <= y + h; }

void Act2Screen::put(sf::RenderTarget& rt, const char* s, float x, float y, unsigned size, sf::Color c) {
	text.setCharacterSize(size);
	text.setString(s);
	text.setFillColor(c);
	text.setOrigin({ 0.0f, 0.0f });
	text.setPosition({ x, y });
	rt.draw(text);
}

void Act2Screen::putWrapped(sf::RenderTarget& rt, const char* s, float x, float y, unsigned size, sf::Color c, int maxChars, float lineH) {
	char line[160];
	int len = tLen(s), i = 0, row = 0;
	while (i < len) {
		int end = i + maxChars;
		if (end >= len) end = len;
		else { int k = end; while (k > i && s[k] != ' ') k--; if (k > i) end = k; }
		int n = end - i; if (n > 159) n = 159;
		for (int j = 0; j < n; j++) line[j] = s[i + j];
		line[n] = 0;
		put(rt, line, x, y + (float)row * lineH, size, c);
		row++;
		i = end;
		while (i < len && s[i] == ' ') i++;
	}
}

void Act2Screen::button(sf::RenderTarget& rt, const char* s, float x, float y, float w, float h, bool enabled) {
	bool hov = enabled && inside(mx, my, x, y, w, h);
	sf::RectangleShape r({ w, h });
	r.setPosition({ x, y });
	r.setFillColor(!enabled ? sf::Color(35, 38, 52) : (hov ? sf::Color(70, 90, 140) : sf::Color(45, 55, 85)));
	r.setOutlineThickness(2.0f);
	r.setOutlineColor(enabled ? sf::Color(150, 165, 210) : sf::Color(70, 74, 95));
	rt.draw(r);
	put(rt, s, x + 14.0f, y + h / 2.0f - 12.0f, 20, enabled ? sf::Color::White : sf::Color(110, 115, 135));
}

// flow 

void Act2Screen::startLevel(int i) {
	sel = i;
	log.clear();
	level.begin(defs[i], &log);
	mirror.clear();
	mirror.loadFromModel(level.tree.getTree());
	mirror.setPendingKey(level.lastInsertKey);
	pb.start(&log, &mirror);
	pb.clearLines();
	view.setTargetId(0); view.setHintId(0);
	mode = PLAY; ended = false; deliveryTimer = 0.8f;
}

void Act2Screen::settleBeforeAction() {
	if (!pb.finished()) pb.finishNow();   // never act while the drawn tree lags behind the model
	log.clear();
	view.setHintId(0);
}

void Act2Screen::afterAction() {
	mirror.setPendingKey(level.lastInsertKey);
	pb.start(&log, &mirror);
}

void Act2Screen::finishLevel() {
	ended = true;
	if (level.state == Act2Level::WON) {
		int s = level.stars();
		if (s > prog.stars[2][sel]) { prog.stars[2][sel] = s; dirty = true; }
		if (level.def.type == LevelType::BOSS) { startCutscene(); return; }
	}
	mode = RESULT;
}

void Act2Screen::startCutscene() {
	int n = level.def.nodes;
	cutAvl.reset();
	for (int k = 1; k <= n; k++) cutAvl.insert(k);
	cutMirror.clear();
	cutMirror.loadFromModel(cutAvl.getTree());
	cutPage = 0;
	mode = CUTSCENE;
}

void Act2Screen::act(bool ok) { if (ok) afterAction(); }

// input 

void Act2Screen::handleEvent(const sf::Event& e) {
	if (const auto* mm = e.getIf<sf::Event::MouseMoved>()) { mx = (float)mm->position.x; my = (float)mm->position.y; return; }

	const auto* mb = e.getIf<sf::Event::MouseButtonPressed>();
	const auto* kp = e.getIf<sf::Event::KeyPressed>();
	if (mb) { mx = (float)mb->position.x; my = (float)mb->position.y; }
	bool click = mb && mb->button == sf::Mouse::Button::Left;

	if (mode == SELECT) {
		if (click) for (int i = 0; i < defCount; i++)
			if (unlocked(i) && inside(mx, my, 140.0f, 140.0f + 84.0f * (float)i, 700.0f, 68.0f)) { sel = i; mode = BRIEF; }
		if (kp) {
			int k = (int)kp->code - (int)sf::Keyboard::Key::Num1;
			if (k >= 0 && k < defCount && unlocked(k)) { sel = k; mode = BRIEF; }
			if (kp->code == sf::Keyboard::Key::Escape) exitFlag = true;
		}
	}
	else if (mode == BRIEF) {
		if (click && inside(mx, my, 140.0f, 560.0f, 300.0f, 60.0f)) startLevel(sel);
		if (click && inside(mx, my, 460.0f, 560.0f, 240.0f, 60.0f)) mode = SELECT;
		if (kp && (kp->code == sf::Keyboard::Key::Enter || kp->code == sf::Keyboard::Key::Space)) startLevel(sel);
		if (kp && kp->code == sf::Keyboard::Key::Escape) mode = SELECT;
	}
	else if (mode == PLAY) {
		if (level.state == Act2Level::PLAYING && level.phase == Act2Level::PH_PLAY) {
			int type = level.def.type;
			bool nav = (type == LevelType::SEARCH || type == LevelType::INSERT || type == LevelType::BOSS);
			if (click) {
				if (type == LevelType::REMOVE) {
					for (int c = 0; c < 3; c++)
						if (inside(mx, my, 40.0f + 270.0f * (float)c, WIN_H - 110.0f, 250.0f, 60.0f)) { settleBeforeAction(); mirror.clearMarks(); act(level.answerCase(c, &log)); }
				}
				else {
					int id = view.hitTest(mirror, mx, my);
					if (id) { settleBeforeAction(); act(level.clickNode(id, &log)); }
				}
			}
			if (kp) {
				if (nav && kp->code == sf::Keyboard::Key::Left) { settleBeforeAction(); act(level.move(1, &log)); }
				if (nav && kp->code == sf::Keyboard::Key::Right) { settleBeforeAction(); act(level.move(2, &log)); }
				if (type == LevelType::REMOVE) {
					int k = (int)kp->code - (int)sf::Keyboard::Key::Num1;
					if (k >= 0 && k < 3) { settleBeforeAction(); mirror.clearMarks(); act(level.answerCase(k, &log)); }
				}
				if (kp->code == sf::Keyboard::Key::H) { level.hint(); view.setHintId(level.hintId); }
			}
		}
		if (kp) {
			if (kp->code == sf::Keyboard::Key::R) startLevel(sel);
			if (kp->code == sf::Keyboard::Key::Escape) mode = SELECT;
			if (kp->code == sf::Keyboard::Key::Space) pb.setPaused(!pb.isPaused());
			if (kp->code == sf::Keyboard::Key::Up) pb.setSpeed(pb.getSpeed() * 1.5f);
			if (kp->code == sf::Keyboard::Key::Down) pb.setSpeed(pb.getSpeed() / 1.5f);
		}
	}
	else if (mode == CUTSCENE) {
		bool next = (kp && (kp->code == sf::Keyboard::Key::Space || kp->code == sf::Keyboard::Key::Enter || kp->code == sf::Keyboard::Key::Right)) || click;
		if (next) { cutPage++; if (cutPage >= 3) mode = RESULT; }
	}
	else if (mode == RESULT) {
		bool won = level.state == Act2Level::WON;
		if (click && inside(mx, my, 140.0f, 560.0f, 220.0f, 60.0f)) startLevel(sel);
		if (click && won && sel + 1 < defCount && inside(mx, my, 380.0f, 560.0f, 220.0f, 60.0f)) startLevel(sel + 1);
		if (click && inside(mx, my, 620.0f, 560.0f, 220.0f, 60.0f)) mode = SELECT;
		if (kp) {
			if (kp->code == sf::Keyboard::Key::R) startLevel(sel);
			if (kp->code == sf::Keyboard::Key::N && won && sel + 1 < defCount) startLevel(sel + 1);
			if (kp->code == sf::Keyboard::Key::Escape) mode = SELECT;
		}
	}
}

void Act2Screen::update(float dt) {
	if (mode != PLAY) return;
	level.tick(dt);
	pb.update(dt);
	mirror.animateStep(dt);

	if (level.phase == Act2Level::PH_DELIVERY && level.state == Act2Level::PLAYING && pb.finished()) {
		deliveryTimer -= dt;
		if (deliveryTimer <= 0.0f) {         // one sorted delivery arrives, instantly applied to the picture
			log.clear();
			level.deliverNext(&log);
			afterAction();
			pb.finishNow();
			deliveryTimer = 0.35f;
		}
	}
	if (level.state != Act2Level::PLAYING && pb.finished() && !ended) finishLevel();
}

// drawing 

void Act2Screen::draw(sf::RenderWindow& w) {
	w.clear(sf::Color(18, 20, 30));
	if (mode == SELECT) drawSelect(w);
	else if (mode == BRIEF) drawBrief(w);
	else if (mode == PLAY) drawPlay(w);
	else if (mode == CUTSCENE) drawCutscene(w);
	else drawResult(w);
}

void Act2Screen::drawSelect(sf::RenderWindow& w) {
	put(w, "ACT 2 - THE SORTED WING (Binary Search Tree)", 140.0f, 40.0f, 30, sf::Color(255, 215, 90));
	put(w, "Click a level or press 1-5.   Esc: back.   F1: sandbox", 140.0f, 90.0f, 16, sf::Color(140, 150, 175));
	for (int i = 0; i < defCount; i++) {
		char s[96]; tCopy(s, 96, defs[i].id); tAppend(s, 96, "   "); tAppend(s, 96, defs[i].title);
		bool ok = unlocked(i);
		button(w, s, 140.0f, 140.0f + 84.0f * (float)i, 700.0f, 68.0f, ok);
		char st[24];
		if (!ok) tCopy(st, 24, "LOCKED");
		else { tCopy(st, 24, "Stars: "); tAppendInt(st, 24, prog.stars[2][i]); tAppend(st, 24, "/3"); }
		put(w, st, 700.0f, 140.0f + 84.0f * (float)i + 22.0f, 18, ok ? sf::Color(255, 215, 90) : sf::Color(110, 115, 135));
	}
}

void Act2Screen::drawBrief(sf::RenderWindow& w) {
	const LevelDef& d = defs[sel];
	char t[80]; tCopy(t, 80, "Level "); tAppend(t, 80, d.id); tAppend(t, 80, " - "); tAppend(t, 80, d.title);
	put(w, t, 140.0f, 60.0f, 30, sf::Color(255, 215, 90));
	putWrapped(w, d.brief, 140.0f, 140.0f, 22, sf::Color(225, 230, 245), 70, 32.0f);
	put(w, "Rules: entering a room or comparing costs 1 alarm. Alarm capacity = ceil(1.5 x optimal) + 2.", 140.0f, 430.0f, 16, sf::Color(150, 160, 185));
	put(w, "Wrong clicks cost 1 alarm and count as a penalty. H = hint (caps stars at 2).", 140.0f, 456.0f, 16, sf::Color(150, 160, 185));
	button(w, "Start (Enter)", 140.0f, 560.0f, 300.0f, 60.0f, true);
	button(w, "Back (Esc)", 460.0f, 560.0f, 240.0f, 60.0f, true);
}

void Act2Screen::drawHud(sf::RenderWindow& w) {
	char t[96]; tCopy(t, 96, "Level "); tAppend(t, 96, level.def.id); tAppend(t, 96, " - "); tAppend(t, 96, level.def.title);
	put(w, t, 14.0f, 8.0f, 22, sf::Color(255, 215, 90));

	char o[120]; o[0] = 0;
	switch (level.def.type) {
	case LevelType::SEARCH: tCopy(o, 120, "Reach the room with key "); tAppendInt(o, 120, level.targetKey); break;
	case LevelType::INSERT: tCopy(o, 120, "Stash key "); tAppendInt(o, 120, level.targetKey); tAppend(o, 120, " where it belongs"); break;
	case LevelType::REMOVE: tCopy(o, 120, "Delete key "); tAppendInt(o, 120, level.targetKey); tAppend(o, 120, " - round "); tAppendInt(o, 120, level.round + 1 > level.def.rounds ? level.def.rounds : level.round + 1); tAppend(o, 120, "/"); tAppendInt(o, 120, level.def.rounds); break;
	case LevelType::RANGE: tCopy(o, 120, "Collect all keys in ["); tAppendInt(o, 120, level.lo); tAppend(o, 120, ", "); tAppendInt(o, 120, level.hi); tAppend(o, 120, "]   collected "); tAppendInt(o, 120, level.collected); tAppend(o, 120, "/"); tAppendInt(o, 120, level.inRangeTotal); break;
	case LevelType::BOSS:
		if (level.phase == Act2Level::PH_DELIVERY) { tCopy(o, 120, "Sorted deliveries arriving... "); tAppendInt(o, 120, level.delivered); tAppend(o, 120, "/"); tAppendInt(o, 120, level.def.nodes); }
		else { tCopy(o, 120, "Find key "); tAppendInt(o, 120, level.targetKey); tAppend(o, 120, " before the alarm capacity shrinks!"); }
		break;
	}
	put(w, o, 14.0f, 38.0f, 20, sf::Color(235, 240, 255));

	// alarm meter
	float cap = (float)(level.capacity > 0 ? level.capacity : 1);
	float frac = (float)level.alarm / cap; if (frac > 1.0f) frac = 1.0f;
	sf::RectangleShape back({ 360.0f, 22.0f }); back.setPosition({ 14.0f, 72.0f });
	back.setFillColor(sf::Color(40, 44, 62)); back.setOutlineThickness(2.0f); back.setOutlineColor(sf::Color(150, 160, 190));
	w.draw(back);
	sf::RectangleShape fill({ 360.0f * frac, 22.0f }); fill.setPosition({ 14.0f, 72.0f });
	fill.setFillColor(frac < 0.6f ? sf::Color(50, 170, 90) : (frac < 0.85f ? sf::Color(220, 180, 40) : sf::Color(220, 60, 60)));
	w.draw(fill);
	char a[96]; tCopy(a, 96, "ALARM "); tAppendInt(a, 96, level.alarm); tAppend(a, 96, " / "); tAppendInt(a, 96, level.capacity);
	tAppend(a, 96, "   par "); tAppendInt(a, 96, level.optimal); tAppend(a, 96, "   penalties "); tAppendInt(a, 96, level.penalties);
	put(w, a, 390.0f, 72.0f, 18, sf::Color(220, 225, 240));
	put(w, level.message, 14.0f, 102.0f, 18, sf::Color(150, 200, 255));
}

void Act2Screen::drawPlay(sf::RenderWindow& w) {
	drawHud(w);
	view.setShowBalance(false);
	view.setTargetId(level.targetNodeId());
	float treeBottom = (level.def.type == LevelType::REMOVE) ? 120.0f : 60.0f;
	view.setArea(10.0f, 140.0f, WIN_W - 340.0f, WIN_H - 140.0f - treeBottom);
	view.draw(w, mirror);

	sf::RectangleShape panel({ 320.0f, WIN_H - 150.0f });
	panel.setPosition({ WIN_W - 330.0f, 140.0f });
	panel.setFillColor(sf::Color(28, 31, 46)); panel.setOutlineThickness(1.0f); panel.setOutlineColor(sf::Color(70, 76, 100));
	w.draw(panel);
	put(w, pb.isPaused() ? "Event log (PAUSED)" : "Event log", WIN_W - 320.0f, 148.0f, 16, sf::Color(220, 225, 240));
	for (int i = 0; i < pb.lineCount(); i++)
		put(w, pb.line(i), WIN_W - 320.0f, 178.0f + 24.0f * (float)i, 14, i == 0 ? sf::Color::White : sf::Color(140, 148, 170));

	if (level.def.type == LevelType::REMOVE) {
		const char* names[3] = { "1  Leaf", "2  One child", "3  Two children" };
		bool on = level.state == Act2Level::PLAYING;
		for (int c = 0; c < 3; c++) button(w, names[c], 40.0f + 270.0f * (float)c, WIN_H - 110.0f, 250.0f, 60.0f, on);
	}
	const char* help = (level.def.type == LevelType::REMOVE) ? "1/2/3 or click: choose the case | H: hint | R: retry | Esc: levels"
		: (level.def.type == LevelType::RANGE) ? "Click rooms next to ones you entered | H: hint | R: retry | Esc: levels"
		: "Left/Right arrow or click a child room | H: hint | R: retry | Esc: levels";
	put(w, help, 14.0f, WIN_H - 28.0f, 14, sf::Color(120, 128, 150));
}

void Act2Screen::drawCutscene(sf::RenderWindow& w) {
	int n = level.def.nodes;
	int hBst = level.tree.height(level.tree.getTree());
	int hAvl = cutAvl.height(cutAvl.getTree());
	char t[220];
	if (cutPage == 0) { tCopy(t, 220, "The Lazy Architect built the vault in the order the keys arrived. They arrived sorted, so every new room went to the right of the last one."); }
	else if (cutPage == 1) {
		tCopy(t, 220, "The vault is a corridor: height "); tAppendInt(t, 220, hBst);
		tAppend(t, 220, " for "); tAppendInt(t, 220, n); tAppend(t, 220, " rooms. Finding the last treasure costs "); tAppendInt(t, 220, n);
		tAppend(t, 220, " steps - the alarm meter fills like a linear search.");
	}
	else {
		tCopy(t, 220, "The same keys in a balanced (AVL) tree have height "); tAppendInt(t, 220, hAvl);
		tAppend(t, 220, ", so any search costs at most "); tAppendInt(t, 220, hAvl);
		tAppend(t, 220, " steps. Balancing keeps height near log2(n). That is Act 3.");
	}
	put(w, "CUTSCENE: why balancing is needed", 40.0f, 24.0f, 26, sf::Color(255, 215, 90));
	putWrapped(w, t, 40.0f, 70.0f, 20, sf::Color(230, 235, 250), 100, 28.0f);
	view.setTargetId(0); view.setHintId(0);
	view.setArea(10.0f, 150.0f, WIN_W - 20.0f, WIN_H - 220.0f);
	if (cutPage < 2) { view.setShowBalance(false); view.draw(w, mirror); }
	else { view.setShowBalance(true); view.draw(w, cutMirror); view.setShowBalance(false); }
	put(w, "Space / click: continue", 40.0f, WIN_H - 36.0f, 16, sf::Color(130, 138, 160));
}

void Act2Screen::drawResult(sf::RenderWindow& w) {
	bool won = level.state == Act2Level::WON;
	put(w, won ? "VAULT CRACKED!" : "ALARM TRIGGERED - RETRY", 140.0f, 50.0f, 34, won ? sf::Color(255, 215, 90) : sf::Color(230, 90, 90));
	int s = level.stars();
	for (int i = 0; i < 3; i++) {
		sf::CircleShape c(28.0f); c.setPosition({ 140.0f + 80.0f * (float)i, 120.0f });
		c.setFillColor(i < s ? sf::Color(255, 205, 60) : sf::Color(45, 48, 66));
		c.setOutlineThickness(2.0f); c.setOutlineColor(sf::Color(200, 205, 220));
		w.draw(c);
	}
	char st[24]; tCopy(st, 24, "Stars: "); tAppendInt(st, 24, s); tAppend(st, 24, "/3");
	put(w, st, 400.0f, 132.0f, 24, sf::Color::White);
	char l[96];
	tCopy(l, 96, "Steps (alarm): "); tAppendInt(l, 96, level.alarm); tAppend(l, 96, "     Optimal: "); tAppendInt(l, 96, level.optimal);
	put(w, l, 140.0f, 220.0f, 22, sf::Color(225, 230, 245));
	tCopy(l, 96, "Penalties: "); tAppendInt(l, 96, level.penalties); tAppend(l, 96, level.hintUsed ? "     Hint used (stars capped at 2)" : "");
	put(w, l, 140.0f, 256.0f, 22, sf::Color(225, 230, 245));
	tCopy(l, 96, "Score: "); tAppendInt(l, 96, level.score());
	if (level.bonus > 0) { tAppend(l, 96, "     (pruning bonus +"); tAppendInt(l, 96, level.bonus); tAppend(l, 96, ")"); }
	put(w, l, 140.0f, 292.0f, 22, sf::Color(255, 215, 90));
	put(w, level.message, 140.0f, 340.0f, 18, sf::Color(150, 200, 255));
	button(w, "Retry (R)", 140.0f, 560.0f, 220.0f, 60.0f, true);
	button(w, "Next (N)", 380.0f, 560.0f, 220.0f, 60.0f, won && sel + 1 < defCount);
	button(w, "Levels (Esc)", 620.0f, 560.0f, 220.0f, 60.0f, true);
}