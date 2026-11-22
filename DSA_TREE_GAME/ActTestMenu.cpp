#include "ActTestMenu.h"


ActTestMenu::ActTestMenu(const sf::Font& f, Act2Screen& a2, Act3Screen& a3)
    : font(f), act2(a2), act3(a3), active(nullptr), title(f, "ACT TEST MENU", 34) {
}

void ActTestMenu::drawButton(sf::RenderWindow& w, const char* text, float y) {
    sf::RectangleShape r({ 520.0f,76.0f });
    r.setPosition({ 380.0f,y });r.setFillColor(sf::Color(38, 48, 72));
    r.setOutlineThickness(2.0f);r.setOutlineColor(sf::Color(120, 145, 190));w.draw(r);
    sf::Text t(font, text, 25);t.setFillColor(sf::Color::White);
    sf::FloatRect b = t.getLocalBounds();t.setOrigin({ b.position.x + b.size.x / 2,b.position.y + b.size.y / 2 });
    t.setPosition({ 640.0f,y + 38.0f });w.draw(t);
}

void ActTestMenu::handleEvent(const sf::Event& e) {
    if (active) {
        active->handleEvent(e);
        if (active == &act2 && act2.exitRequested()) { act2.clearExit();active = nullptr; }
        if (active == &act3 && act3.exitRequested()) { act3.clearExit();active = nullptr; }
        if (const sf::Event::KeyPressed* k = e.getIf<sf::Event::KeyPressed>())
            if (k->code == sf::Keyboard::Key::Escape && active == nullptr) return;
        return;
    }
    if (const sf::Event::KeyPressed* k = e.getIf<sf::Event::KeyPressed>()) {
        if (k->code == sf::Keyboard::Key::Num1) active = &act2;
        else if (k->code == sf::Keyboard::Key::Num2) active = &act3;
    }
    if (const sf::Event::MouseButtonPressed* m = e.getIf<sf::Event::MouseButtonPressed>()) {
        if (m->button == sf::Mouse::Button::Left) {
            float x = (float)m->position.x * (2.0f / 3.0f);
            float y = (float)m->position.y * (2.0f / 3.0f);
            if (x >= 380 && x <= 900 && y >= 260 && y <= 336) active = &act2;
            if (x >= 380 && x <= 900 && y >= 370 && y <= 446) active = &act3;
        }
    }
}
void ActTestMenu::update(float dt) { if (active) active->update(dt); }
void ActTestMenu::draw(sf::RenderWindow& w) {
    if (active) { active->draw(w);return; }
    w.clear(sf::Color(17, 21, 34));
    title.setFillColor(sf::Color(255, 215, 100));
    sf::FloatRect b = title.getLocalBounds();title.setOrigin({ b.position.x + b.size.x / 2,b.position.y + b.size.y / 2 });
    title.setPosition({ 640,145 });w.draw(title);
    sf::Text sub(font, "Choose an act to test", 20);sub.setFillColor(sf::Color(175, 185, 205));
    sub.setPosition({ 510,195 });w.draw(sub);
    drawButton(w, "1  -  ACT 2: THE SORTED WING (BST)", 260);
    drawButton(w, "2  -  ACT 3: THE ARCHITECT'S WING (AVL)", 370);
    sf::Text help(font, "Press 1 or 2 to launch. Press Esc inside an act to return.", 16);
    help.setFillColor(sf::Color(145, 155, 175));help.setPosition({ 360,500 });w.draw(help);
}
