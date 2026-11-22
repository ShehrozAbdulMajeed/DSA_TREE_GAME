#pragma once
#include <SFML/Graphics.hpp>
#include "TreeMirror.h"

// Shared tree interaction layer used by Acts 2 and 3.
class TreeView {
private:
    struct Map { float ox, oy, cellW, cellH, radius; };
    sf::Text label;
    float ax, ay, aw, ah;
    bool showBalance, showSize, dragging;
    int targetId, hintId, selectedId, hoverId;
    float zoom, panX, panY, lastMouseX, lastMouseY, hoverX, hoverY;
    bool mapOf(const TreeMirror& m, Map& mp) const;
    void drawLabel(sf::RenderTarget& rt, const char* s, unsigned size, float cx, float cy, sf::Color c);
public:
    TreeView(const sf::Font& font);
    void setArea(float x, float y, float w, float h) { ax = x; ay = y; aw = w; ah = h; }
    void setShowBalance(bool b) { showBalance = b; }
    void setTargetId(int id) { targetId = id; }
    void setHintId(int id) { hintId = id; }
    void setSelectedId(int id) { selectedId = id; }
    void setShowSize(bool b) { showSize = b; }
    // Call for every event before a screen handles its own mouse input.
    void handleEvent(const sf::Event& e);
    void resetView() { zoom = 1.0f; panX = 0.0f; panY = 0.0f; }
    int hitTest(const TreeMirror& m, float mx, float my) const;
    void draw(sf::RenderTarget& rt, const TreeMirror& m);
};