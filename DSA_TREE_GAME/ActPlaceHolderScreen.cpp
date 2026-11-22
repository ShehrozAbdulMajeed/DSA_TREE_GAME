#include "ActPlaceholderScreen.h"


ActPlaceholderScreen::ActPlaceholderScreen(const sf::Font& f, const char* t)
    : font(f), actTitle(t), heading(f, "", 32), body(f, "", 20), back(false) {
}
void ActPlaceholderScreen::handleEvent(const sf::Event& e) {
    if (const sf::Event::KeyPressed* k = e.getIf<sf::Event::KeyPressed>()) if (k->code == sf::Keyboard::Key::Escape) back = true;
}
void ActPlaceholderScreen::draw(sf::RenderWindow& w) {
    w.clear(sf::Color(18, 20, 30));
    heading.setString(actTitle); heading.setFillColor(sf::Color(255, 215, 90));
    heading.setPosition({ 80.f, 90.f }); w.draw(heading);
    body.setString("INTEGRATION PLACEHOLDER\n\nYour groupmate can replace this screen with their own GUI.\nThe Act 1 / Act 4 visual design does not need to match Acts 2 or 3.\n\nPress ESC to return to the campaign map.");
    body.setPosition({ 80.f, 190.f }); body.setFillColor(sf::Color(220, 225, 240)); w.draw(body);
}
