#pragma once
// Act 3 - The Architect's Wing (AVL)
#include "Screen.h"
#include "Act3Level.h"
#include "Playback.h"
#include "TreeView.h"
#include "Progress.h"
#include "SaveManager.h"

class Act3Screen : public Screen {
public:
	static const int SELECT = 0, BRIEF = 1, PLAY = 2, RESULT = 3;
private:
	Progress& prog;
	LevelDef defs[16];
	int defCount;
	Act3Level level;
	EventLog log;
	TreeMirror mirror;
	Playback pb;
	TreeView view;
	sf::Text text;
	int mode, sel, selectedId;
	float mx, my, deliveryTimer, reloadTimer;
	bool ended, exitFlag, dirty, sizeCheckPending, sizeBad, submitted;
	char sizeNote[96];
	LeaderEntry board[5];
	int boardCount;

	bool unlocked(int i) const;
	void startLevel(int i);
	void afterAction();
	void settleBeforeAction();
	void finishLevel();
	void rotateSelected(bool left);
	void put(sf::RenderTarget& rt, const char* s, float x, float y, unsigned size, sf::Color c);
	void putWrapped(sf::RenderTarget& rt, const char* s, float x, float y, unsigned size, sf::Color c, int maxChars, float lineH);
	void button(sf::RenderTarget& rt, const char* s, float x, float y, float w, float h, bool enabled);
	static bool inside(float px, float py, float x, float y, float w, float h);
	void drawHud(sf::RenderWindow& w);
	void drawSelect(sf::RenderWindow& w);
	void drawBrief(sf::RenderWindow& w);
	void drawPlay(sf::RenderWindow& w);
	void drawResult(sf::RenderWindow& w);
public:
	Act3Screen(const sf::Font& font, Progress& progress, int gameSeed);
	void handleEvent(const sf::Event& e) override;
	void update(float dt) override;
	void draw(sf::RenderWindow& w) override;
	bool exitRequested() const { return exitFlag; }
	void clearExit() { exitFlag = false; mode = SELECT; }
	bool progressChanged() { bool d = dirty; dirty = false; return d; }
};