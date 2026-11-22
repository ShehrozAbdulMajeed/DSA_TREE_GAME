#pragma once
#include <SFML/Graphics.hpp>
#include "TreeMirror.h"

class TreeView {
private:
	struct Map { float ox, oy, cellW, cellH, radius; };
	sf::Text label;
	float ax, ay, aw, ah;
	bool showBalance;
	int targetId, hintId;
	bool mapOf(const TreeMirror& m, Map& mp) const;
	void drawLabel(sf::RenderTarget& rt, const char* s, unsigned size, float cx, float cy, sf::Color c);
public:
	TreeView(const sf::Font& font);
	void setArea(float x, float y, float w, float h) { ax = x; ay = y; aw = w; ah = h; }
	void setShowBalance(bool b) { showBalance = b; }   // AVL badges (Act 3)
	void setTargetId(int id) { targetId = id; }        // gold ring + "TARGET" tag
	void setHintId(int id) { hintId = id; }            // cyan ring
	int hitTest(const TreeMirror& m, float mx, float my) const;   // node id under the mouse, 0 = none
	void draw(sf::RenderTarget& rt, const TreeMirror& m);
};