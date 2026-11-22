#include "NavigationScreen.h"

NavigationScreen::NavigationScreen(const sf::Font& f)
    : font(f), campaign(false), choice(NONE), title(f, "", 38) {
}

void NavigationScreen::drawButton(sf::RenderTarget& w, const char* label, float y) {
    sf::RectangleShape r({ 430.f, 58.f });
    r.setPosition({ 425.f, y });
    r.setFillColor(sf::Color(38, 51, 76));
    r.setOutlineThickness(2.f);
    r.setOutlineColor(sf::Color(190, 165, 95));
    w.draw(r);
    sf::Text t(font, label, 23);
    sf::FloatRect b = t.getLocalBounds();
    t.setOrigin({ b.position.x + b.size.x / 2.f, b.position.y + b.size.y / 2.f });
    t.setPosition({ 640.f, y + 29.f });
    t.setFillColor(sf::Color(245, 235, 210));
    w.draw(t);
}

void NavigationScreen::handleEvent(const sf::Event& e) {
    if (const sf::Event::KeyPressed* k = e.getIf<sf::Event::KeyPressed>()) {
        if (k->code == sf::Keyboard::Key::Escape && campaign) { campaign = false; return; }
        if (!campaign) {
            if (k->code == sf::Keyboard::Key::Enter || k->code == sf::Keyboard::Key::C) choice = CAMPAIGN;
            if (k->code == sf::Keyboard::Key::S) choice = SANDBOX;
            if (k->code == sf::Keyboard::Key::Q) choice = QUIT;
        }
        else {
            if (k->code == sf::Keyboard::Key::Num1) choice = ACT1;
            if (k->code == sf::Keyboard::Key::Num2) choice = ACT2;
            if (k->code == sf::Keyboard::Key::Num3) choice = ACT3;
            if (k->code == sf::Keyboard::Key::Num4) choice = ACT4;
        }
    }
    if (const sf::Event::MouseButtonPressed* m = e.getIf<sf::Event::MouseButtonPressed>()) {
        if (m->button != sf::Mouse::Button::Left) return;
        float x = (float)m->position.x, y = (float)m->position.y;
        if (x < 425.f || x > 855.f) return;
        int row = (int)((y - 220.f) / 78.f);
        if (!campaign) {
            if (row == 0) choice = CAMPAIGN;
            else if (row == 1) choice = SANDBOX;
            else if (row == 2) choice = QUIT;
        }
        else if (row >= 0 && row <= 3) choice = (Choice)(ACT1 + row);
    }
}

void NavigationScreen::draw(sf::RenderWindow& w) {
    w.clear(sf::Color(16, 20, 31));
    title.setString(campaign ? "CAMPAIGN MAP" : "VAULT HEIST: THE LIVING VAULT");
    title.setFillColor(sf::Color(255, 215, 110));
    sf::FloatRect b = title.getLocalBounds();
    title.setOrigin({ b.position.x + b.size.x / 2.f, b.position.y + b.size.y / 2.f });
    title.setPosition({ 640.f, 125.f });
    w.draw(title);
    if (campaign) {
        drawButton(w, "ACT 1 - THE BLUEPRINT WING   [1]", 220.f);
        drawButton(w, "ACT 2 - THE SORTED WING      [2]", 298.f);
        drawButton(w, "ACT 3 - THE ARCHITECT'S WING [3]", 376.f);
        drawButton(w, "ACT 4 - THE HEAP FORTRESS    [4]", 454.f);
        sf::Text hint(font, "ESC: Main Menu", 17);
        hint.setPosition({ 550.f, 570.f }); w.draw(hint);
    }
    else {
        drawButton(w, "CAMPAIGN  [ENTER / C]", 220.f);
        drawButton(w, "SANDBOX  [S]", 298.f);
        drawButton(w, "QUIT  [Q]", 376.f);
    }
}
