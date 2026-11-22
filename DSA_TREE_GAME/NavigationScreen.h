#pragma once
#include "Screen.h"

// Shared navigation only. Act GUIs remain independent.
class NavigationScreen : public Screen {
public:
    enum Choice { NONE, CAMPAIGN, SANDBOX, ACT1, ACT2, ACT3, ACT4, QUIT };
private:
    const sf::Font& font;
    bool campaign;
    Choice choice;
    sf::Text title;
    void drawButton(sf::RenderTarget& w, const char* label, float y);
public:
    explicit NavigationScreen(const sf::Font& f);
    void handleEvent(const sf::Event& e) override;
    void update(float) override {}
    void draw(sf::RenderWindow& w) override;
    Choice takeChoice() { Choice c = choice; choice = NONE; return c; }
    void showCampaign(bool b) { campaign = b; choice = NONE; }
};
