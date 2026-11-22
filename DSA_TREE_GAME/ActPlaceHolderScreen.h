#pragma once
#include "Screen.h"

// Temporary integration point. Replace this class with the group member's
// own Act 1 / Act 4 Screen implementation when their files are merged.
// No shared GUI layout is imposed on either act.
class ActPlaceholderScreen : public Screen {
    const sf::Font& font;
    const char* actTitle;
    sf::Text heading, body;
    bool back;
public:
    ActPlaceholderScreen(const sf::Font& f, const char* title);
    void handleEvent(const sf::Event& e) override;
    void update(float) override {}
    void draw(sf::RenderWindow& w) override;
    bool backRequested() const { return back; }
};
