#include "SandboxScreen.h"
#include "TextUtil.h"
#include "SaveManager.h"

SandboxScreen::SandboxScreen(const sf::Font& font) : view(font), text(font, "", 16) {
	useAVL = false; cur = &bst;
	inputLen = 0; input[0] = 0; op = 0; rngState = 2026; wasPlaying = false;
	tCopy(status, 96, "Type a number, press Enter.");
	view.setShowBalance(false);
}

int SandboxScreen::rnd() { rngState = rngState * 1103515245u + 12345u; return (int)((rngState >> 16) & 0x7fff); }

void SandboxScreen::put(sf::RenderTarget& rt, const char* s, float x, float y, unsigned size, sf::Color c) {
	text.setCharacterSize(size);
	text.setString(s);
	text.setFillColor(c);
	text.setOrigin({ 0.0f, 0.0f });
	text.setPosition({ x, y });
	rt.draw(text);
}

void SandboxScreen::verifySync() {
	if (!mirror.matchesModel(cur->getTree()))
		tCopy(status, 96, "SYNC ERROR: drawn tree differs from model!");   // dev aid
}

void SandboxScreen::runOp(int key) {
	if (!pb.finished()) { pb.finishNow(); verifySync(); }   // never start while the mirror is behind the model
	mirror.clearMarks();
	log.clear();
	pb.clearLines();
	mirror.setPendingKey(key);

	bool ok = false;
	status[0] = 0;
	if (op == 0) {
		ok = cur->insert(key, &log);
		tCopy(status, 96, ok ? "Inserted " : "Duplicate rejected: ");
		tAppendInt(status, 96, key);
	}
	else if (op == 1) {
		ok = cur->remove(key, &log);
		tCopy(status, 96, ok ? "Deleted " : "Key not found: ");
		tAppendInt(status, 96, key);
	}
	else {
		ok = (cur->search(key, &log) != nullptr);
		tCopy(status, 96, ok ? "Found " : "Not found: ");
		tAppendInt(status, 96, key);
	}
	pb.start(&log, &mirror);
	wasPlaying = true;
}

void SandboxScreen::randomInsert() { runOp(rnd() % 100); }

void SandboxScreen::clearTree() {
	pb.finishNow();
	bst.setTree(nullptr);     // setTree deletes the old tree
	avl.setTree(nullptr);
	mirror.clear();
	log.clear();
	pb.start(&log, &mirror);
	pb.clearLines();
	tCopy(status, 96, "Cleared.");
}

void SandboxScreen::switchTree() {
	clearTree();
	useAVL = !useAVL;
	cur = useAVL ? static_cast<BST*>(&avl) : &bst;
	view.setShowBalance(useAVL);
	tCopy(status, 96, useAVL ? "Switched to AVL (tree cleared)." : "Switched to BST (tree cleared).");
}

void SandboxScreen::saveSandbox() {
	pb.finishNow();
	Progress p;
	char err[96];
	int r = SaveManager::saveGame("sandbox.sav", p, (int)rngState, cur, useAVL ? SaveManager::KIND_AVL : SaveManager::KIND_BST, nullptr, err, 96);
	tCopy(status, 96, r == SaveResult::OK ? "Saved to sandbox.sav" : err);
}

void SandboxScreen::loadSandbox() {
	pb.finishNow();
	Progress p; int seed = 0, kind = 0; bool hasHeap = false; char err[96];
	int r = SaveManager::loadGame("sandbox.sav", p, seed, &bst, &avl, kind, nullptr, hasHeap, err, 96);
	if (r == SaveResult::NOT_FOUND) { tCopy(status, 96, "No sandbox.sav yet - press F5 to save first."); return; }
	if (r != SaveResult::OK) { tCopy(status, 96, "Load rejected: "); tAppend(status, 96, err); return; }
	if (kind == SaveManager::KIND_NONE) { tCopy(status, 96, "That file holds no tree."); return; }
	useAVL = (kind == SaveManager::KIND_AVL);
	if (useAVL) bst.reset(); else avl.reset();           // only one tree is active
	cur = useAVL ? static_cast<BST*>(&avl) : &bst;
	view.setShowBalance(useAVL);
	log.clear();
	mirror.loadFromModel(cur->getTree());
	pb.start(&log, &mirror);
	pb.clearLines();
	tCopy(status, 96, "Loaded sandbox.sav (validated).");
}

void SandboxScreen::handleEvent(const sf::Event& e) {
	if (const auto* te = e.getIf<sf::Event::TextEntered>()) {
		unsigned c = te->unicode;
		bool digit = (c >= '0' && c <= '9');
		bool minus = (c == '-' && inputLen == 0);
		if ((digit || minus) && inputLen < 9) { input[inputLen++] = (char)c; input[inputLen] = 0; }
	}
	else if (const auto* kp = e.getIf<sf::Event::KeyPressed>()) {
		switch (kp->code) {
		case sf::Keyboard::Key::Backspace:
			if (inputLen > 0) input[--inputLen] = 0;
			break;
		case sf::Keyboard::Key::Enter: {
			if (inputLen == 0 || (inputLen == 1 && input[0] == '-')) { tCopy(status, 96, "Type a number first."); break; }
			int v = 0, i = 0; bool neg = false;
			if (input[0] == '-') { neg = true; i = 1; }
			for (; i < inputLen; i++) v = v * 10 + (input[i] - '0');
			runOp(neg ? -v : v);
			inputLen = 0; input[0] = 0;
			break;
		}
		case sf::Keyboard::Key::Tab:    op = (op + 1) % 3; break;
		case sf::Keyboard::Key::T:      switchTree(); break;
		case sf::Keyboard::Key::R:      randomInsert(); break;
		case sf::Keyboard::Key::C:      clearTree(); break;
		case sf::Keyboard::Key::F5:     saveSandbox(); break;
		case sf::Keyboard::Key::F9:     loadSandbox(); break;
		case sf::Keyboard::Key::Space:  pb.setPaused(!pb.isPaused()); break;
		case sf::Keyboard::Key::Right:  pb.setPaused(true); pb.step(); break;
		case sf::Keyboard::Key::Up:     pb.setSpeed(pb.getSpeed() * 1.5f); break;
		case sf::Keyboard::Key::Down:   pb.setSpeed(pb.getSpeed() / 1.5f); break;
		default: break;
		}
	}
}

void SandboxScreen::update(float dt) {
	pb.update(dt);
	mirror.animateStep(dt);
	if (wasPlaying && pb.finished()) { wasPlaying = false; verifySync(); }
}

void SandboxScreen::draw(sf::RenderWindow& w) {
	float W = (float)w.getSize().x, H = (float)w.getSize().y;
	w.clear(sf::Color(18, 20, 30));

	view.setArea(10.0f, 100.0f, W - 340.0f, H - 150.0f);
	view.draw(w, mirror);

	// HUD
	const char* opName = op == 0 ? "INSERT" : (op == 1 ? "DELETE" : "SEARCH");
	char l1[96]; tCopy(l1, 96, useAVL ? "Tree: AVL" : "Tree: BST");
	tAppend(l1, 96, "    Mode: "); tAppend(l1, 96, opName);
	tAppend(l1, 96, "    Nodes: "); tAppendInt(l1, 96, mirror.count());
	tAppend(l1, 96, "    Alarm steps: "); tAppendInt(l1, 96, pb.alarmSteps());
	if (useAVL) { tAppend(l1, 96, "    Rotations: "); tAppendInt(l1, 96, pb.rotations); }
	put(w, l1, 14.0f, 10.0f, 18, sf::Color(220, 225, 240));

	char l2[48]; tCopy(l2, 48, "Key: "); tAppend(l2, 48, input); tAppend(l2, 48, "_");
	put(w, l2, 14.0f, 38.0f, 22, sf::Color(255, 215, 90));
	put(w, status, 14.0f, 70.0f, 16, sf::Color(150, 200, 255));

	// Event Log panel
	sf::RectangleShape panel({ 320.0f, H - 20.0f });
	panel.setPosition({ W - 330.0f, 10.0f });
	panel.setFillColor(sf::Color(28, 31, 46));
	panel.setOutlineThickness(1.0f);
	panel.setOutlineColor(sf::Color(70, 76, 100));
	w.draw(panel);
	char sp[40]; tCopy(sp, 40, pb.isPaused() ? "Event log (PAUSED) speed x" : "Event log   speed x");
	tAppendInt(sp, 40, (int)(pb.getSpeed() * 10.0f + 0.5f) / 10);
	tAppend(sp, 40, ".");
	tAppendInt(sp, 40, (int)(pb.getSpeed() * 10.0f + 0.5f) % 10);
	put(w, sp, W - 320.0f, 18.0f, 16, sf::Color(220, 225, 240));
	for (int i = 0; i < pb.lineCount(); i++)
		put(w, pb.line(i), W - 320.0f, 50.0f + 24.0f * (float)i, 14, i == 0 ? sf::Color::White : sf::Color(140, 148, 170));

	put(w, "Digits+Enter: run | Tab: insert/delete/search | T: BST/AVL | R: random | C: clear | F5/F9: save/load | Space: pause | Right: step | Up/Down: speed",
		14.0f, H - 28.0f, 13, sf::Color(120, 128, 150));
}