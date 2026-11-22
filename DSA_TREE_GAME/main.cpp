#include <SFML/Graphics.hpp>
#include <iostream>
#include "Act2Screen.h"
#include "Act3Screen.h"
#include "ActTestMenu.h"
#include "Progress.h"
#include "SaveManager.h"

int main() {
    sf::RenderWindow window(
        sf::VideoMode({ 1920, 1080 }),
        "Vault Heist - Act 2 / Act 3 Test Menu"
    );

    sf::View gameView(
        sf::FloatRect({ 0.f, 0.f }, { 1280.f, 720.f })
    );

    window.setView(gameView);
    window.setFramerateLimit(60);
    sf::Font font;
    if (!font.openFromFile("assets/font.ttf") && !font.openFromFile("C:/Windows/Fonts/arial.ttf")) {
        std::cout << "Could not load a font. Put a .ttf at assets/font.ttf\n"; return 1;
    }
    const char* SAVE_PATH = "savegame.sav";
    Progress progress; int seed = 2026, treeKind = 0; bool hasHeap = false; char err[96];
    int loaded = SaveManager::loadGame(SAVE_PATH, progress, seed, nullptr, nullptr, treeKind, nullptr, hasHeap, err, 96);
    if (loaded == SaveResult::INVALID) std::cout << "Save file rejected (" << err << "). Starting fresh.\n";
    Act2Screen act2(font, progress);
    Act3Screen act3(font, progress);
    ActTestMenu menu(font, act2, act3);
    sf::Clock clock;
    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) { window.close();continue; }
            menu.handleEvent(*event);
        }
        float dt = clock.restart().asSeconds();
        menu.update(dt);
        if (act2.progressChanged() | act3.progressChanged())
            SaveManager::saveGame(SAVE_PATH, progress, seed, nullptr, SaveManager::KIND_NONE, nullptr, err, 96);
        menu.draw(window);window.display();
    }
    SaveManager::saveGame(SAVE_PATH, progress, seed, nullptr, SaveManager::KIND_NONE, nullptr, err, 96);
    return 0;
}
