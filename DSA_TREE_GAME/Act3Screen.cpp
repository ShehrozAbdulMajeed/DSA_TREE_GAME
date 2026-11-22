#include "Act3Screen.h"
#include "LevelFile.h"
#include "TextUtil.h"

static const float WIN_W = 1280.0f, WIN_H = 720.0f;
static const char* LEADERBOARD_PATH = "leaderboard.txt";

Act3Screen::Act3Screen(const sf::Font& font, Progress& progress)
	: prog(progress), view(font), text(font, "", 16) {
	LevelDef all[24];
	int n = loadLevelDefs(all, 24);
	defCount = 0;
	for (int i = 0; i < n && defCount < 16; i++) if (all[i].act == 3) defs[defCount++] = all[i];
	mode = SELECT; sel = 0; selectedId = 0; mx = my = 0; deliveryTimer = 0; reloadTimer = 0.7f;
	ended = false; exitFlag = false; dirty = false; sizeCheckPending = false; sizeBad = false; submitted = false;
	sizeNote[0] = 0; boardCount = 0;
}

bool Act3Screen::unlocked(int i) const { return i == 0 || prog.stars[3][i - 1] > 0; }
bool Act3Screen::inside(float px, float py, float x, float y, float w, float h) { return px >= x && px <= x + w && py >= y && py <= y + h; }

void Act3Screen::put(sf::RenderTarget& rt, const char* s, float x, float y, unsigned size, sf::Color c) {
	text.setCharacterSize(size);
	text.setString(s);
	text.setFillColor(c);
	text.setOrigin({ 0.0f, 0.0f });
	text.setPosition({ x, y });
	rt.draw(text);
}

void Act3Screen::putWrapped(sf::RenderTarget& rt, const char* s, float x, float y, unsigned size, sf::Color c, int maxChars, float lineH) {
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

void Act3Screen::button(sf::RenderTarget& rt, const char* s, float x, float y, float w, float h, bool enabled) {
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

void Act3Screen::startLevel(int i) {
	sel = i;
	log.clear();
	level.begin(defs[i], &log);
	mirror.clear();
	mirror.loadFromModel(level.tree.getTree());
	mirror.setPendingKey(level.lastInsertKey);
	pb.start(&log, &mirror);
	pb.clearLines();
	pb.setSpeed(level.def.type == LevelType::ORDER ? 3.0f : 1.0f);     
	view.setTargetId(0); view.setHintId(0); view.setSelectedId(0);
	selectedId = 0; mode = PLAY; ended = false; submitted = false;
	deliveryTimer = 0.8f; reloadTimer = 0.7f; sizeCheckPending = false; sizeBad = false; sizeNote[0] = 0;
}

void Act3Screen::settleBeforeAction() {
	if (!pb.finished()) pb.finishNow();
	log.clear();
	view.setHintId(0);
}

void Act3Screen::afterAction() {
	mirror.setPendingKey(level.lastInsertKey);
	pb.start(&log, &mirror);
}

void Act3Screen::rotateSelected(bool left) {
	if (selectedId == 0) { tCopy(level.message, 128, "Click a room first to select it."); return; }
	settleBeforeAction();
	bool ok = level.rotate(selectedId, left, &log);
	if (ok) afterAction();
}

void Act3Screen::finishLevel() {
	ended = true;
	int s = level.stars();
	if ((level.state == Act3Level::WON || level.def.type == LevelType::ENDLESS) && s > prog.stars[3][sel]) {
		prog.stars[3][sel] = s;
		dirty = true;
	}
	if (level.def.type == LevelType::ENDLESS && !submitted) {
		submitted = true;
		Leaderboard::submit(LEADERBOARD_PATH, "PLAYER", level.score());
		boardCount = Leaderboard::load(LEADERBOARD_PATH, board, 5);
	}
	mode = RESULT;
}

// input 

void Act3Screen::handleEvent(const sf::Event& e) {
	if (const auto* mm = e.getIf<sf::Event::MouseMoved>()) { mx = (float)mm->position.x; my = (float)mm->position.y; return; }

	const auto* mb = e.getIf<sf::Event::MouseButtonPressed>();
	const auto* kp = e.getIf<sf::Event::KeyPressed>();
	if (mb) { mx = (float)mb->position.x; my = (float)mb->position.y; }
	bool click = mb && mb->button == sf::Mouse::Button::Left;

	if (mode == SELECT) {
		if (click) for (int i = 0; i < defCount; i++)
			if (unlocked(i) && inside(mx, my, 140.0f, 130.0f + 78.0f * (float)i, 700.0f, 64.0f)) { sel = i; mode = BRIEF; }
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
		int type = level.def.type;
		bool live = level.state == Act3Level::PLAYING;
		if (live && click) {
			if (type == LevelType::SINGLE || type == LevelType::DOUBLE || type == LevelType::ENDLESS) {
				if (inside(mx, my, 40.0f, WIN_H - 110.0f, 250.0f, 60.0f)) rotateSelected(true);
				else if (inside(mx, my, 310.0f, WIN_H - 110.0f, 250.0f, 60.0f)) rotateSelected(false);
				else if (type == LevelType::DOUBLE && !level.identified) {
					for (int c = 0; c < 4; c++)
						if (inside(mx, my, 600.0f + 130.0f * (float)c, WIN_H - 110.0f, 120.0f, 60.0f)) { settleBeforeAction(); if (level.answerCase(c, &log)) afterAction(); }
				}
				if (!inside(mx, my, 0.0f, WIN_H - 120.0f, WIN_W, 120.0f)) {
					int id = view.hitTest(mirror, mx, my);
					if (id) selectedId = id;
				}
			}
			else if (type == LevelType::PREDICT) {
				const int map[5] = { Act3Case::NONE, Act3Case::LL, Act3Case::RR, Act3Case::LR, Act3Case::RL };
				for (int b = 0; b < 5; b++)
					if (inside(mx, my, 40.0f + 190.0f * (float)b, WIN_H - 110.0f, 175.0f, 60.0f)) {
						settleBeforeAction();
						if (level.predict(map[b], &log)) afterAction();
					}
			}
			else if (type == LevelType::BADGES || type == LevelType::ORDER) {
				int id = view.hitTest(mirror, mx, my);
				if (id) { settleBeforeAction(); if (level.clickNode(id, &log)) afterAction(); }
			}
		}
		if (kp) {
			if (live) {
				if (type == LevelType::ORDER && level.phase == Act3Level::PH_PLAY) {
					if (kp->code == sf::Keyboard::Key::Left) { settleBeforeAction(); if (level.move(1, &log)) afterAction(); }
					if (kp->code == sf::Keyboard::Key::Right) { settleBeforeAction(); if (level.move(2, &log)) afterAction(); }
				}
				if (type == LevelType::SINGLE || type == LevelType::DOUBLE || type == LevelType::ENDLESS) {
					if (kp->code == sf::Keyboard::Key::Left) rotateSelected(true);
					if (kp->code == sf::Keyboard::Key::Right) rotateSelected(false);
				}
				if (type == LevelType::DOUBLE && !level.identified) {
					int k = (int)kp->code - (int)sf::Keyboard::Key::Num1;
					if (k >= 0 && k < 4) { settleBeforeAction(); if (level.answerCase(k, &log)) afterAction(); }
				}
				if (type == LevelType::PREDICT) {
					const int map[5] = { Act3Case::NONE, Act3Case::LL, Act3Case::RR, Act3Case::LR, Act3Case::RL };
					int k = (int)kp->code - (int)sf::Keyboard::Key::Num1;
					if (k >= 0 && k < 5) { settleBeforeAction(); if (level.predict(map[k], &log)) afterAction(); }
				}
				if (kp->code == sf::Keyboard::Key::H) { level.hint(); view.setHintId(level.hintId); }
			}
			if (kp->code == sf::Keyboard::Key::R) startLevel(sel);
			if (kp->code == sf::Keyboard::Key::Escape) mode = SELECT;
			if (kp->code == sf::Keyboard::Key::Space) pb.setPaused(!pb.isPaused());
			if (kp->code == sf::Keyboard::Key::Up) pb.setSpeed(pb.getSpeed() * 1.5f);
			if (kp->code == sf::Keyboard::Key::Down) pb.setSpeed(pb.getSpeed() / 1.5f);
		}
	}
	else if (mode == RESULT) {
		bool won = level.state == Act3Level::WON;
		bool endless = level.def.type == LevelType::ENDLESS;
		bool canNext = (won || endless) && sel + 1 < defCount;
		if (click && inside(mx, my, 140.0f, 600.0f, 220.0f, 60.0f)) startLevel(sel);
		if (click && canNext && inside(mx, my, 380.0f, 600.0f, 220.0f, 60.0f)) startLevel(sel + 1);
		if (click && inside(mx, my, 620.0f, 600.0f, 220.0f, 60.0f)) mode = SELECT;
		if (kp) {
			if (kp->code == sf::Keyboard::Key::R) startLevel(sel);
			if (kp->code == sf::Keyboard::Key::N && canNext) startLevel(sel + 1);
			if (kp->code == sf::Keyboard::Key::Escape) mode = SELECT;
		}
	}
}

void Act3Screen::update(float dt) {
	if (mode != PLAY) return;
	int type = level.def.type;

	if (type == LevelType::ENDLESS && level.state == Act3Level::PLAYING) {
		if (pb.finished()) { log.clear(); if (level.tick(dt, &log)) afterAction(); }
		else level.tick(dt, nullptr);                  
	}
	pb.update(dt);
	mirror.animateStep(dt);

	if (level.pendingReload && pb.finished() && level.state == Act3Level::PLAYING) {   // next round
		reloadTimer -= dt;
		if (reloadTimer <= 0.0f) {
			mirror.loadFromModel(level.tree.getTree());
			level.pendingReload = false;
			selectedId = 0; reloadTimer = 0.7f;
			view.setHintId(0);
			pb.clearLines();
		}
	}

	if (type == LevelType::ORDER && level.phase == Act3Level::PH_DELIVERY && level.state == Act3Level::PLAYING && pb.finished()) {
		deliveryTimer -= dt;
		if (deliveryTimer <= 0.0f) {
			log.clear();
			level.deliverNext(&log);
			afterAction();
			deliveryTimer = 0.25f;
			if (level.phase == Act3Level::PH_PLAY) { pb.setSpeed(1.0f); sizeCheckPending = true; }
		}
	}
	if (sizeCheckPending && pb.finished()) {           
		sizeCheckPending = false;
		int bad = mirror.firstBadAnnotation();
		sizeBad = bad != 0;
		if (bad) tCopy(sizeNote, 96, "BOSS CHECK FAILED: a room has a wrong size or height!");
		else {
			tCopy(sizeNote, 96, "Boss check passed: every size is correct after ");
			tAppendInt(sizeNote, 96, level.deliveryRotations);
			tAppend(sizeNote, 96, " rotations.");
		}
	}
	if (level.state != Act3Level::PLAYING && pb.finished() && !ended) finishLevel();
}

// drawing 

void Act3Screen::draw(sf::RenderWindow& w) {
	w.clear(sf::Color(18, 20, 30));
	if (mode == SELECT) drawSelect(w);
	else if (mode == BRIEF) drawBrief(w);
	else if (mode == PLAY) drawPlay(w);
	else drawResult(w);
}

void Act3Screen::drawSelect(sf::RenderWindow& w) {
	put(w, "ACT 3 - THE ARCHITECT'S WING (AVL Tree)", 140.0f, 36.0f, 30, sf::Color(255, 215, 90));
	put(w, "Click a level or press 1-6.   F2: Act 2   F1: sandbox", 140.0f, 84.0f, 16, sf::Color(140, 150, 175));
	for (int i = 0; i < defCount; i++) {
		char s[96]; tCopy(s, 96, defs[i].id); tAppend(s, 96, "   "); tAppend(s, 96, defs[i].title);
		bool ok = unlocked(i);
		button(w, s, 140.0f, 130.0f + 78.0f * (float)i, 700.0f, 64.0f, ok);
		char st[24];
		if (!ok) tCopy(st, 24, "LOCKED");
		else { tCopy(st, 24, "Stars: "); tAppendInt(st, 24, prog.stars[3][i]); tAppend(st, 24, "/3"); }
		put(w, st, 700.0f, 130.0f + 78.0f * (float)i + 20.0f, 18, ok ? sf::Color(255, 215, 90) : sf::Color(110, 115, 135));
	}
}

void Act3Screen::drawBrief(sf::RenderWindow& w) {
	const LevelDef& d = defs[sel];
	char t[80]; tCopy(t, 80, "Level "); tAppend(t, 80, d.id); tAppend(t, 80, " - "); tAppend(t, 80, d.title);
	put(w, t, 140.0f, 60.0f, 30, sf::Color(255, 215, 90));
	putWrapped(w, d.brief, 140.0f, 140.0f, 22, sf::Color(225, 230, 245), 70, 32.0f);
	if (d.type != LevelType::ENDLESS) {
		put(w, "Rules: each step costs 1 alarm. Capacity = ceil(1.5 x optimal) + 2.", 140.0f, 430.0f, 16, sf::Color(150, 160, 185));
		put(w, "Wrong rotation or prediction: +2 alarm and -50 points. H = hint (caps stars at 2).", 140.0f, 456.0f, 16, sf::Color(150, 160, 185));
	}
	else put(w, "Stars by survival time: 30 s = 1, 60 s = 2, 120 s = 3. Score = seconds survived.", 140.0f, 430.0f, 16, sf::Color(150, 160, 185));
	button(w, "Start (Enter)", 140.0f, 560.0f, 300.0f, 60.0f, true);
	button(w, "Back (Esc)", 460.0f, 560.0f, 240.0f, 60.0f, true);
}

void Act3Screen::drawHud(sf::RenderWindow& w) {
	char t[96]; tCopy(t, 96, "Level "); tAppend(t, 96, level.def.id); tAppend(t, 96, " - "); tAppend(t, 96, level.def.title);
	put(w, t, 14.0f, 8.0f, 22, sf::Color(255, 215, 90));

	char o[140]; o[0] = 0;
	switch (level.def.type) {
	case LevelType::BADGES: tCopy(o, 140, "Click every unbalanced room.  Found "); tAppendInt(o, 140, level.found); tAppend(o, 140, "/"); tAppendInt(o, 140, level.unbalancedTotal); break;
	case LevelType::SINGLE: case LevelType::DOUBLE:
		tCopy(o, 140, "Repair "); tAppendInt(o, 140, level.round + 1 > level.def.rounds ? level.def.rounds : level.round + 1); tAppend(o, 140, "/"); tAppendInt(o, 140, level.def.rounds);
		tAppend(o, 140, level.identified ? ": select a room, then rotate it" : ": identify the case first (1=LL 2=RR 3=LR 4=RL)");
		break;
	case LevelType::PREDICT: tCopy(o, 140, "Insert "); tAppendInt(o, 140, level.round + 1 > level.def.rounds ? level.def.rounds : level.round + 1); tAppend(o, 140, "/"); tAppendInt(o, 140, level.def.rounds); tAppend(o, 140, ": predict the rotation for key "); tAppendInt(o, 140, level.pendingKey); break;
	case LevelType::ORDER:
		if (level.phase == Act3Level::PH_DELIVERY) { tCopy(o, 140, "Sorted deliveries arriving and rebalancing... "); tAppendInt(o, 140, level.delivered); tAppend(o, 140, "/"); tAppendInt(o, 140, level.def.nodes); }
		else { tCopy(o, 140, "Steal the "); tAppendInt(o, 140, level.kWanted); tAppend(o, 140, level.kWanted == 1 ? "st" : (level.kWanted == 2 ? "nd" : (level.kWanted == 3 ? "rd" : "th"))); tAppend(o, 140, " most valuable (largest). Looking for number "); tAppendInt(o, 140, level.kRemain); tAppend(o, 140, " from the smallest in this subtree"); }
		break;
	case LevelType::ENDLESS: tCopy(o, 140, "Survive!  Select a room and rotate to repair damage. Collapse at balance 3."); break;
	}
	put(w, o, 14.0f, 38.0f, 19, sf::Color(235, 240, 255));

	char a[120];
	if (level.def.type == LevelType::ENDLESS) {
		tCopy(a, 120, "SURVIVED "); tAppendInt(a, 120, (int)level.survived); tAppend(a, 120, " s    operations: "); tAppendInt(a, 120, level.opsApplied);
		tAppend(a, 120, "    unbalanced rooms: "); tAppendInt(a, 120, level.unbalancedCount());
		put(w, a, 14.0f, 72.0f, 20, sf::Color(255, 215, 90));
	}
	else {
		float cap = (float)(level.capacity > 0 ? level.capacity : 1);
		float frac = (float)level.alarm / cap; if (frac > 1.0f) frac = 1.0f;
		sf::RectangleShape back({ 360.0f, 22.0f }); back.setPosition({ 14.0f, 72.0f });
		back.setFillColor(sf::Color(40, 44, 62)); back.setOutlineThickness(2.0f); back.setOutlineColor(sf::Color(150, 160, 190));
		w.draw(back);
		sf::RectangleShape fill({ 360.0f * frac, 22.0f }); fill.setPosition({ 14.0f, 72.0f });
		fill.setFillColor(frac < 0.6f ? sf::Color(50, 170, 90) : (frac < 0.85f ? sf::Color(220, 180, 40) : sf::Color(220, 60, 60)));
		w.draw(fill);
		tCopy(a, 120, "ALARM "); tAppendInt(a, 120, level.alarm); tAppend(a, 120, " / "); tAppendInt(a, 120, level.capacity);
		tAppend(a, 120, "   par "); tAppendInt(a, 120, level.optimal); tAppend(a, 120, "   penalties "); tAppendInt(a, 120, level.penalties);
		put(w, a, 390.0f, 72.0f, 18, sf::Color(220, 225, 240));
	}
	put(w, level.message, 14.0f, 102.0f, 18, sf::Color(150, 200, 255));
}

void Act3Screen::drawPlay(sf::RenderWindow& w) {
	drawHud(w);
	int type = level.def.type;
	view.setShowBalance(true);
	view.setShowSize(type == LevelType::ORDER);
	view.setTargetId(0);
	view.setSelectedId(selectedId);
	bool buttonsRow = (type == LevelType::SINGLE || type == LevelType::DOUBLE || type == LevelType::ENDLESS || type == LevelType::PREDICT);
	float bottom = buttonsRow ? 120.0f : 60.0f;
	view.setArea(10.0f, 140.0f, WIN_W - 340.0f, WIN_H - 140.0f - bottom);
	view.draw(w, mirror);

	sf::RectangleShape panel({ 320.0f, WIN_H - 150.0f });
	panel.setPosition({ WIN_W - 330.0f, 140.0f });
	panel.setFillColor(sf::Color(28, 31, 46)); panel.setOutlineThickness(1.0f); panel.setOutlineColor(sf::Color(70, 76, 100));
	w.draw(panel);
	put(w, pb.isPaused() ? "Event log (PAUSED)" : "Event log", WIN_W - 320.0f, 148.0f, 16, sf::Color(220, 225, 240));
	for (int i = 0; i < pb.lineCount(); i++)
		put(w, pb.line(i), WIN_W - 320.0f, 178.0f + 24.0f * (float)i, 14, i == 0 ? sf::Color::White : sf::Color(140, 148, 170));
	if (sizeNote[0]) put(w, sizeNote, WIN_W - 320.0f, 392.0f, 14, sizeBad ? sf::Color(240, 90, 90) : sf::Color(90, 210, 130));

	bool live = level.state == Act3Level::PLAYING;
	if (type == LevelType::SINGLE || type == LevelType::DOUBLE || type == LevelType::ENDLESS) {
		button(w, "Rotate Left  (<-)", 40.0f, WIN_H - 110.0f, 250.0f, 60.0f, live);
		button(w, "Rotate Right (->)", 310.0f, WIN_H - 110.0f, 250.0f, 60.0f, live);
		if (type == LevelType::DOUBLE) {
			const char* nm[4] = { "1 LL", "2 RR", "3 LR", "4 RL" };
			for (int c = 0; c < 4; c++) button(w, nm[c], 600.0f + 130.0f * (float)c, WIN_H - 110.0f, 120.0f, 60.0f, live && !level.identified);
		}
	}
	else if (type == LevelType::PREDICT) {
		const char* nm[5] = { "1 None", "2 LL", "3 RR", "4 LR", "5 RL" };
		for (int b = 0; b < 5; b++) button(w, nm[b], 40.0f + 190.0f * (float)b, WIN_H - 110.0f, 175.0f, 60.0f, live);
	}
	const char* help = (type == LevelType::BADGES) ? "Click unbalanced (red) rooms | H: hint | R: retry | Esc: levels"
		: (type == LevelType::ORDER) ? "Left/Right arrow or click a child room | H: hint | R: retry | Esc: levels"
		: (type == LevelType::PREDICT) ? "1-5 or click: predict | H: hint | R: retry | Esc: levels"
		: "Click a room to select | Left/Right arrow or buttons rotate | H: hint | R: retry | Esc: levels";
	put(w, help, 14.0f, WIN_H - 28.0f, 14, sf::Color(120, 128, 150));
}

void Act3Screen::drawResult(sf::RenderWindow& w) {
	bool endless = level.def.type == LevelType::ENDLESS;
	bool won = level.state == Act3Level::WON;
	if (endless) put(w, "THE VAULT COLLAPSED", 140.0f, 50.0f, 34, sf::Color(230, 150, 90));
	else put(w, won ? "VAULT BALANCED!" : "ALARM TRIGGERED - RETRY", 140.0f, 50.0f, 34, won ? sf::Color(255, 215, 90) : sf::Color(230, 90, 90));
	int s = level.stars();
	for (int i = 0; i < 3; i++) {
		sf::CircleShape c(28.0f); c.setPosition({ 140.0f + 80.0f * (float)i, 120.0f });
		c.setFillColor(i < s ? sf::Color(255, 205, 60) : sf::Color(45, 48, 66));
		c.setOutlineThickness(2.0f); c.setOutlineColor(sf::Color(200, 205, 220));
		w.draw(c);
	}
	char st[24]; tCopy(st, 24, "Stars: "); tAppendInt(st, 24, s); tAppend(st, 24, "/3");
	put(w, st, 400.0f, 132.0f, 24, sf::Color::White);
	char l[110];
	if (endless) {
		tCopy(l, 110, "Survived: "); tAppendInt(l, 110, (int)level.survived); tAppend(l, 110, " seconds     operations handled: "); tAppendInt(l, 110, level.opsApplied);
		put(w, l, 140.0f, 220.0f, 22, sf::Color(225, 230, 245));
		tCopy(l, 110, "Score: "); tAppendInt(l, 110, level.score());
		put(w, l, 140.0f, 256.0f, 22, sf::Color(255, 215, 90));
		put(w, "Leaderboard (best scores)", 140.0f, 310.0f, 20, sf::Color(220, 225, 240));
		for (int i = 0; i < boardCount; i++) {
			tCopy(l, 110, ""); tAppendInt(l, 110, i + 1); tAppend(l, 110, ".  "); tAppend(l, 110, board[i].name); tAppend(l, 110, "   "); tAppendInt(l, 110, board[i].score);
			put(w, l, 140.0f, 345.0f + 28.0f * (float)i, 18, sf::Color(190, 198, 220));
		}
	}
	else {
		tCopy(l, 110, "Steps (alarm): "); tAppendInt(l, 110, level.alarm); tAppend(l, 110, "     Optimal: "); tAppendInt(l, 110, level.optimal);
		put(w, l, 140.0f, 220.0f, 22, sf::Color(225, 230, 245));
		tCopy(l, 110, "Penalties: "); tAppendInt(l, 110, level.penalties); tAppend(l, 110, level.hintUsed ? "     Hint used (stars capped at 2)" : "");
		put(w, l, 140.0f, 256.0f, 22, sf::Color(225, 230, 245));
		tCopy(l, 110, "Score: "); tAppendInt(l, 110, level.score());
		put(w, l, 140.0f, 292.0f, 22, sf::Color(255, 215, 90));
		put(w, level.message, 140.0f, 340.0f, 18, sf::Color(150, 200, 255));
	}
	bool canNext = (won || endless) && sel + 1 < defCount;
	button(w, "Retry (R)", 140.0f, 600.0f, 220.0f, 60.0f, true);
	button(w, "Next (N)", 380.0f, 600.0f, 220.0f, 60.0f, canNext);
	button(w, "Levels (Esc)", 620.0f, 600.0f, 220.0f, 60.0f, true);
}