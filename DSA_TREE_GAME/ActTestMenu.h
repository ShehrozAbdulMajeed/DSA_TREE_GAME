#pragma once
#include "Screen.h"
#include "Act2Screen.h"
#include "Act3Screen.h"

// Small standalone launcher for testing the two implemented acts.
class ActTestMenu : public Screen {
    const sf::Font& font;
    Act2Screen& act2;
    Act3Screen& act3;
    Screen* active;
    sf::Text title;
    void drawButton(sf::RenderWindow& w, const char* text, float y);
public:
    ActTestMenu(const sf::Font& f, Act2Screen& a2, Act3Screen& a3);
    void handleEvent(const sf::Event& e) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& w) override;
};
