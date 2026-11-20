#pragma once
#include "Screen.h"
#include "AVL.h"
#include "Playback.h"
#include "TreeView.h"

class SandboxScreen : public Screen {
private:
	BST bst;
	AVL avl;
	bool useAVL;
	BST* cur;               

	EventLog log;
	TreeMirror mirror;
	Playback pb;
	TreeView view;
	sf::Text text;

	char input[12];
	int inputLen;
	int op;                 
	char status[96];
	unsigned int rngState;
	bool wasPlaying;

	int rnd();
	void runOp(int key);
	void randomInsert();
	void clearTree();
	void switchTree();
	void verifySync();
	void put(sf::RenderTarget& rt, const char* s, float x, float y, unsigned size, sf::Color c);
public:
	SandboxScreen(const sf::Font& font);
	void handleEvent(const sf::Event& e) override;
	void update(float dt) override;
	void draw(sf::RenderWindow& w) override;
};