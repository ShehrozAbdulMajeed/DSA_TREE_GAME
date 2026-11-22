#pragma once
// Common interface so every screen (Sandbox, Act 1, Act 2, Act 3, Act 4, menus) plugs into main.cpp the same way.
#include <SFML/Graphics.hpp>

class Screen {
public:
	virtual ~Screen() {}
	virtual void handleEvent(const sf::Event& e) = 0;
	virtual void update(float dt) = 0;
	virtual void draw(sf::RenderWindow& w) = 0;
};