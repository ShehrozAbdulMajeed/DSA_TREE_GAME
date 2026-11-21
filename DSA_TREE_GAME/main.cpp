#define _CRT_SECURE_NO_WARNINGS
#include <SFML/Graphics.hpp>
#include <iostream>
#include "SandboxScreen.h"
#include "Act2Screen.h"
#include "Progress.h"
#include "SaveManager.h"

//this is huzaifa's branch

int main()
{
    sf::RenderWindow window(sf::VideoMode({ 1280, 720 }), "Vault Heist: The Living Vault");
    window.setFramerateLimit(60);

    // Put any .ttf at <working folder>/assets/font.ttf (Windows Arial is the fallback).
    sf::Font font;
    if (!font.openFromFile("assets/font.ttf") && !font.openFromFile("C:/Windows/Fonts/arial.ttf"))
    {
        std::cout << "Could not load a font. Put a .ttf at assets/font.ttf\n";
        return 1;
    }

    // ---- save file: progress is loaded at start and saved after every level and on exit ----
    const char* SAVE_PATH = "savegame.sav";
    Progress progress;                       // shared by every act's screens
    int seed = 2026, treeKind = 0;
    bool hasHeap = false;
    char err[96];
    int loaded = SaveManager::loadGame(SAVE_PATH, progress, seed, nullptr, nullptr, treeKind, nullptr, hasHeap, err, 96);
    if (loaded == SaveResult::INVALID)
        std::cout << "Save file rejected (" << err << "). Starting fresh.\n";
    else if (loaded == SaveResult::OK)
        std::cout << "Progress loaded.\n";
    SandboxScreen sandbox(font);
    Act2Screen act2(font, progress);
    Screen* active = &act2;                  // later: a ScreenManager + main menu + campaign map

    sf::Clock clock;
    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
            else if (const auto* kp = event->getIf<sf::Event::KeyPressed>();
                kp && kp->code == sf::Keyboard::Key::F1)
                active = &sandbox;
            else if (kp && kp->code == sf::Keyboard::Key::F2)
                active = &act2;
            else
                active->handleEvent(*event);
        }

        float dt = clock.restart().asSeconds();
        active->update(dt);
        if (act2.progressChanged())
            SaveManager::saveGame(SAVE_PATH, progress, seed, nullptr, SaveManager::KIND_NONE, nullptr, err, 96);
        active->draw(window);
        window.display();
    }
    SaveManager::saveGame(SAVE_PATH, progress, seed, nullptr, SaveManager::KIND_NONE, nullptr, err, 96);
}