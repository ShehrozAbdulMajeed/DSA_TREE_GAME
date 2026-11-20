#include "Playback.h"
#include "TextUtil.h"

Playback::Playback() {
	log = nullptr; mirror = nullptr; next = 0; timer = 0; speed = 1.0f; paused = false;
	lineHead = 0; lineTotal = 0; visits = compares = rotations = 0;
	for (int i = 0; i < 8; i++) lines[i][0] = 0;
}

void Playback::start(const EventLog* l, TreeMirror* m) {
	log = l; mirror = m; next = 0; timer = 0;
	visits = compares = rotations = 0;
}

float Playback::duration(int type) {
	switch (type) {
	case EventTypes::COMPARE: case EventTypes::VISIT: return 0.45f;
	case EventTypes::INSERT: case EventTypes::UNLINK: return 0.6f;
	case EventTypes::SWAP_VALUE: return 0.7f;
	case EventTypes::ROTATE: return 0.9f;
	case EventTypes::BALANCE_ALERT: return 0.6f;
	case EventTypes::HIGHLIGHT_SUBTREE: return 0.25f;
	case EventTypes::UPDATE_HEIGHT: case EventTypes::UPDATE_SIZE: return 0.12f;
	default: return 0.0f;
	}
}

void Playback::pushLine(const char* s) {
	tCopy(lines[lineHead], 96, s);
	lineHead = (lineHead + 1) % 8;
	lineTotal++;
}

const char* Playback::line(int i) const {
	if (i < 0 || i >= lineCount()) return "";
	return lines[(lineHead - 1 - i + 16) % 8];
}

void Playback::describe(const Events& e, char* b, int cap) const {
	b[0] = 0;
	switch (e.type) {
	case EventTypes::COMPARE:
		tAppend(b, cap, "Compare "); tAppendInt(b, cap, e.extra);
		tAppend(b, cap, " with "); tAppendInt(b, cap, mirror->keyOf(e.id));
		tAppend(b, cap, e.data < 0 ? " -> go left" : (e.data > 0 ? " -> go right" : " -> match"));
		break;
	case EventTypes::VISIT:
		tAppend(b, cap, "Visit "); tAppendInt(b, cap, mirror->keyOf(e.id));
		break;
	case EventTypes::INSERT:
		tAppend(b, cap, "Insert "); tAppendInt(b, cap, mirror->getPendingKey());
		if (e.extra == 0) tAppend(b, cap, " as root");
		else {
			tAppend(b, cap, e.data == 1 ? " as left child of " : " as right child of ");
			tAppendInt(b, cap, mirror->keyOf(e.extra));
		}
		break;
	case EventTypes::UNLINK:
		tAppend(b, cap, "Unlink node "); tAppendInt(b, cap, mirror->keyOf(e.id));
		break;
	case EventTypes::SWAP_VALUE:
		tAppend(b, cap, "Replace "); tAppendInt(b, cap, mirror->keyOf(e.id));
		tAppend(b, cap, " with successor "); tAppendInt(b, cap, e.data);
		break;
	case EventTypes::ROTATE:
		tAppend(b, cap, "Rotate ");
		tAppend(b, cap, e.extra == RotationType::LL ? "LL" : e.extra == RotationType::RR ? "RR" : e.extra == RotationType::LR ? "LR" : "RL");
		tAppend(b, cap, " at "); tAppendInt(b, cap, mirror->keyOf(e.id));
		break;
	case EventTypes::UPDATE_HEIGHT:
		tAppend(b, cap, "Height of "); tAppendInt(b, cap, mirror->keyOf(e.id));
		tAppend(b, cap, ": "); tAppendInt(b, cap, e.extra); tAppend(b, cap, " -> "); tAppendInt(b, cap, e.data);
		break;
	case EventTypes::UPDATE_SIZE:
		tAppend(b, cap, "Size of "); tAppendInt(b, cap, mirror->keyOf(e.id));
		tAppend(b, cap, ": "); tAppendInt(b, cap, e.extra); tAppend(b, cap, " -> "); tAppendInt(b, cap, e.data);
		break;
	case EventTypes::BALANCE_ALERT:
		tAppend(b, cap, "Unbalanced at "); tAppendInt(b, cap, mirror->keyOf(e.id));
		tAppend(b, cap, " (balance "); tAppendInt(b, cap, e.extra); tAppend(b, cap, ")");
		break;
	case EventTypes::HIGHLIGHT_SUBTREE:
		tAppend(b, cap, e.extra == 1 ? "Skip subtree at " : e.extra == 2 ? "In range: " : "Look at ");
		tAppendInt(b, cap, mirror->keyOf(e.id));
		break;
	case EventTypes::DONE:
		tAppend(b, cap, "Done");
		break;
	default:
		tAppend(b, cap, "event "); tAppendInt(b, cap, e.type);
		break;
	}
}

void Playback::applyNext() {
	const Events& e = log->get(next++);
	char buf[96];
	describe(e, buf, 96);          
	pushLine(buf);
	if (e.type == EventTypes::VISIT) visits++;
	if (e.type == EventTypes::COMPARE) compares++;
	if (e.type == EventTypes::ROTATE) rotations += (e.extra >= RotationType::LR) ? 2 : 1;
	mirror->apply(e);
	timer = duration(e.type);
}

void Playback::update(float dt) {
	if (finished() || paused) return;
	timer -= dt * speed;
	while (!finished() && timer <= 0.0f) applyNext();
}

void Playback::step() {
	if (!finished()) applyNext();
}

void Playback::finishNow() {
	while (!finished()) applyNext();
}