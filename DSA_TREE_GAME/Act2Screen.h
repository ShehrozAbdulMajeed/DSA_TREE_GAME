#pragma once
// Act 2 - The Sorted Wing (BST): level select, briefing, play, boss cutscene, result.
#include "Screen.h"
#include "AVL.h"
#include "Act2Level.h"
#include "Playback.h"
#include "TreeView.h"
#include "Progress.h"

class Act2Screen : public Screen {
public:
	static const int SELECT = 0, BRIEF = 1, PLAY = 2, CUTSCENE = 3, RESULT = 4;
private:
	Progress& prog;
	LevelDef defs[16];
	int defCount;
	Act2Level level;
	EventLog log;
	TreeMirror mirror, cutMirror;
	AVL cutAvl;
	Playback pb;
	TreeView view;
	sf::Text text;
	int mode, sel, cutPage;
	float mx, my, deliveryTimer;
	bool ended, exitFlag, dirty;

	bool unlocked(int i) const;
	void startLevel(int i);
	void afterAction();
	void settleBeforeAction();
	void finishLevel();
	void startCutscene();
	void act(bool ok);
	void put(sf::RenderTarget& rt, const char* s, float x, float y, unsigned size, sf::Color c);
	void putWrapped(sf::RenderTarget& rt, const char* s, float x, float y, unsigned size, sf::Color c, int maxChars, float lineH);
	void button(sf::RenderTarget& rt, const char* s, float x, float y, float w, float h, bool enabled);
	static bool inside(float px, float py, float x, float y, float w, float h);
	void drawHud(sf::RenderWindow& w);
	void drawSelect(sf::RenderWindow& w);
	void drawBrief(sf::RenderWindow& w);
	void drawPlay(sf::RenderWindow& w);
	void drawCutscene(sf::RenderWindow& w);
	void drawResult(sf::RenderWindow& w);
public:
	Act2Screen(const sf::Font& font, Progress& progress, int gameSeed);
	void handleEvent(const sf::Event& e) override;
	void update(float dt) override;
	void draw(sf::RenderWindow& w) override;
	bool exitRequested() const { return exitFlag; }
	bool progressChanged() { bool d = dirty; dirty = false; return d; }   // main saves when true
	void clearExit() { exitFlag = false; mode = SELECT; }
};