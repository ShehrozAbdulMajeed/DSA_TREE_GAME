#pragma once
// Draws a TreeMirror. Reads only the mirror, so every tree screen can reuse it.
#include <SFML/Graphics.hpp>
#include "TreeMirror.h"

class TreeView {
private:
	sf::Text label;
	float ax, ay, aw, ah;
	bool showBalance;
	void drawLabel(sf::RenderTarget& rt, const char* s, unsigned size, float cx, float cy, sf::Color c);
public:
	TreeView(const sf::Font& font);
	void setArea(float x, float y, float w, float h) { ax = x; ay = y; aw = w; ah = h; }
	void setShowBalance(bool b) { showBalance = b; }   // AVL badges (Act 3)
	void draw(sf::RenderTarget& rt, const TreeMirror& m);
};