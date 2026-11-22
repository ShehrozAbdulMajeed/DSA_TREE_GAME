#pragma once

#include "TreeMirror.h"

class Playback {
private:
	const EventLog* log;
	TreeMirror* mirror;
	int next;
	float timer, speed;
	bool paused;
	char lines[8][96];
	int lineHead, lineTotal;

	void applyNext();
	void pushLine(const char* s);
	void describe(const Events& e, char* buf, int cap) const;
	static float duration(int type);
public:
	int visits, compares, rotations;

	Playback();
	void start(const EventLog* l, TreeMirror* m);
	void update(float dt);
	void step();
	void finishNow();
	bool finished() const { return log == nullptr || next >= log->size(); }
	int alarmSteps() const { return visits + compares; }
	void setPaused(bool p) { paused = p; }
	bool isPaused() const { return paused; }
	void setSpeed(float s) { speed = s < 0.25f ? 0.25f : (s > 8.0f ? 8.0f : s); }
	float getSpeed() const { return speed; }
	int lineCount() const { return lineTotal < 8 ? lineTotal : 8; }
	const char* line(int i) const;
	void clearLines() { lineHead = 0; lineTotal = 0; }
};