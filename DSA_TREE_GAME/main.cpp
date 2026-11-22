#define _CRT_SECURE_NO_WARNINGS
#include <SFML/Graphics.hpp>
#include <iostream>
#include <cstring>
#include "Act2Screen.h"
#include "Act3Screen.h"
#include "ActTestMenu.h"
#include "Progress.h"
#include "SaveManager.h"
#include "Act2Screen.cpp"
#include "Act3Screen.cpp"

static int createNewSeed(sf::Clock& seedClock, unsigned int& newGameCount, int previousSeed) {
    unsigned int value = (unsigned int)seedClock.getElapsedTime().asMicroseconds();
    newGameCount++;
    value ^= newGameCount * 2654435761u;
    if (previousSeed >= 0) value ^= (unsigned int)previousSeed * 2246822519u;
    value ^= value >> 16;
    value *= 2246822519u;
    value ^= value >> 13;
    int result = (int)(value & 0x7fffffffU);
    if (result == 0) result = (int)newGameCount + 1;
    if (result == previousSeed) {
        result++;
        if (result <= 0) result = 1;
    }
    return result;
}

static int getPreviousSavedSeed(const char* savePath) {
    Progress oldProgress;
    int oldSeed = -1;
    int treeKind = 0;
    bool hasHeap = false;
    char error[96];
    int loaded = SaveManager::loadGame(savePath, oldProgress, oldSeed, nullptr, nullptr,
        treeKind, nullptr, hasHeap, error, 96);
    return loaded == SaveResult::OK ? oldSeed : -1;
}

static void drawMenuButton(sf::RenderWindow& window, const sf::Font& font, const char* label, float y) {
    sf::RectangleShape button({ 430.f, 62.f });
    button.setPosition({ 425.f, y });
    button.setFillColor(sf::Color(38, 51, 76));
    button.setOutlineThickness(2.f);
    button.setOutlineColor(sf::Color(190, 165, 95));
    window.draw(button);

    sf::Text text(font, label, 24);
    sf::FloatRect bounds = text.getLocalBounds();
    text.setOrigin({ bounds.position.x + bounds.size.x / 2.f, bounds.position.y + bounds.size.y / 2.f });
    text.setPosition({ 640.f, y + 31.f });
    text.setFillColor(sf::Color(245, 235, 210));
    window.draw(text);
}

int main() {
    sf::RenderWindow window(
        sf::VideoMode({ 1920, 1080 }),
        "Vault Heist - The Living Vault"
    );

    sf::View gameView(sf::FloatRect({ 0.f, 0.f }, { 1280.f, 720.f }));
    window.setView(gameView);
    window.setFramerateLimit(60);

    sf::Font font;
    if (!font.openFromFile("assets/font.ttf") && !font.openFromFile("C:/Windows/Fonts/arial.ttf")) {
        std::cout << "Could not load a font. Put a .ttf at assets/font.ttf\n";
        return 1;
    }

    const char* SAVE_PATH = "savegame.sav";
    Progress progress;
    int seed = 0;
    int treeKind = 0;
    bool hasHeap = false;
    char err[96];
    bool inGame = false;
    bool showMessage = false;
    char message[128] = "";
    sf::Clock seedClock;
    unsigned int newGameCount = 0;

    Act2Screen* act2 = nullptr;
    Act3Screen* act3 = nullptr;
    ActTestMenu* menu = nullptr;
    sf::Clock clock;

    while (window.isOpen()) {
        while (const std::optional<sf::Event> event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
                continue;
            }

            if (inGame) {
                menu->handleEvent(*event);
                if (act2->progressChanged() || act3->progressChanged()) {
                    int saved = SaveManager::saveGame(SAVE_PATH, progress, seed, nullptr,
                        SaveManager::KIND_NONE, nullptr, err, 96);
                    if (saved != SaveResult::OK) std::cout << "Save failed: " << err << '\n';
                }
            }
            else {
                if (const sf::Event::KeyPressed* key = event->getIf<sf::Event::KeyPressed>()) {
                    if (key->code == sf::Keyboard::Key::N || key->code == sf::Keyboard::Key::Enter) {
                        progress = Progress();
                        seed = createNewSeed(seedClock, newGameCount, getPreviousSavedSeed(SAVE_PATH));
                        int saved = SaveManager::saveGame(SAVE_PATH, progress, seed, nullptr,
                            SaveManager::KIND_NONE, nullptr, err, 96);
                        if (saved != SaveResult::OK) {
                            showMessage = true;
                            std::strncpy(message, "Could not create the new save file.", 127);
                            message[127] = 0;
                        }
                        else {
                            showMessage = false;
                            inGame = true;
                        }
                    }
                    else if (key->code == sf::Keyboard::Key::C) {
                        int loaded = SaveManager::loadGame(SAVE_PATH, progress, seed, nullptr, nullptr,
                            treeKind, nullptr, hasHeap, err, 96);
                        if (loaded == SaveResult::OK) {
                            showMessage = false;
                            inGame = true;
                        }
                        else {
                            showMessage = true;
                            if (loaded == SaveResult::NOT_FOUND)
                                std::strncpy(message, "No saved game found. Choose New Game first.", 127);
                            else
                                std::strncpy(message, "The save file is invalid. Choose New Game to restart.", 127);
                            message[127] = 0;
                        }
                    }
                    else if (key->code == sf::Keyboard::Key::Q || key->code == sf::Keyboard::Key::Escape) {
                        window.close();
                    }
                }

                if (const sf::Event::MouseButtonPressed* mouse = event->getIf<sf::Event::MouseButtonPressed>()) {
                    if (mouse->button == sf::Mouse::Button::Left) {
                        float x = (float)mouse->position.x * (2.0f / 3.0f);
                        float y = (float)mouse->position.y * (2.0f / 3.0f);
                        if (x >= 425.f && x <= 855.f) {
                            if (y >= 220.f && y <= 282.f) {
                                progress = Progress();
                                seed = createNewSeed(seedClock, newGameCount, getPreviousSavedSeed(SAVE_PATH));
                                int saved = SaveManager::saveGame(SAVE_PATH, progress, seed, nullptr,
                                    SaveManager::KIND_NONE, nullptr, err, 96);
                                if (saved == SaveResult::OK) { showMessage = false; inGame = true; }
                                else {
                                    showMessage = true;
                                    std::strncpy(message, "Could not create the new save file.", 127);
                                    message[127] = 0;
                                }
                            }
                            else if (y >= 310.f && y <= 372.f) {
                                int loaded = SaveManager::loadGame(SAVE_PATH, progress, seed, nullptr, nullptr,
                                    treeKind, nullptr, hasHeap, err, 96);
                                if (loaded == SaveResult::OK) { showMessage = false; inGame = true; }
                                else {
                                    showMessage = true;
                                    if (loaded == SaveResult::NOT_FOUND)
                                        std::strncpy(message, "No saved game found. Choose New Game first.", 127);
                                    else
                                        std::strncpy(message, "The save file is invalid. Choose New Game to restart.", 127);
                                    message[127] = 0;
                                }
                            }
                            else if (y >= 400.f && y <= 462.f) window.close();
                        }
                    }
                }
            }

            if (!window.isOpen()) break;
            if (inGame && menu == nullptr) {
                act2 = new Act2Screen(font, progress, seed);
                act3 = new Act3Screen(font, progress, seed);
                menu = new ActTestMenu(font, *act2, *act3);
            }
        }

        float dt = clock.restart().asSeconds();
        if (inGame && menu) {
            menu->update(dt);
            if (act2->progressChanged() || act3->progressChanged())
                SaveManager::saveGame(SAVE_PATH, progress, seed, nullptr, SaveManager::KIND_NONE, nullptr, err, 96);
            menu->draw(window);
        }
        else {
            window.clear(sf::Color(16, 20, 31));
            sf::Text title(font, "VAULT HEIST: THE LIVING VAULT", 38);
            title.setFillColor(sf::Color(255, 215, 110));
            sf::FloatRect bounds = title.getLocalBounds();
            title.setOrigin({ bounds.position.x + bounds.size.x / 2.f, bounds.position.y + bounds.size.y / 2.f });
            title.setPosition({ 640.f, 135.f });
            window.draw(title);
            drawMenuButton(window, font, "NEW GAME  [N / ENTER]", 220.f);
            drawMenuButton(window, font, "CONTINUE  [C]", 310.f);
            drawMenuButton(window, font, "QUIT  [Q / ESC]", 400.f);
            if (showMessage) {
                sf::Text notice(font, message, 17);
                notice.setFillColor(sf::Color(255, 170, 130));
                sf::FloatRect nb = notice.getLocalBounds();
                notice.setOrigin({ nb.position.x + nb.size.x / 2.f, nb.position.y + nb.size.y / 2.f });
                notice.setPosition({ 640.f, 520.f });
                window.draw(notice);
            }
        }
        window.display();
    }

    if (inGame)
        SaveManager::saveGame(SAVE_PATH, progress, seed, nullptr, SaveManager::KIND_NONE, nullptr, err, 96);
    delete menu;
    delete act2;
    delete act3;
    return 0;
}
