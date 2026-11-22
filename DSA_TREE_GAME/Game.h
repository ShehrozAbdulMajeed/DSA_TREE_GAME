#pragma once

#include <SFML/Graphics.hpp>

#include "Screen.h"
#include "Progress.h"
#include "Act1Screen.h"
#include "Act2Screen.h"
#include "Act4Screen.h"

class Game {
private:
    static const int MENU = 0;
    static const int CAMPAIGN = 1;
    static const int PLAYING = 2;
    static const int SANDBOX = 3;

    sf::RenderWindow window;
    sf::Font font;

    Progress progress;

    Screen* activeScreen;

    Act1Screen* act1;
    Act2Screen* act2;
    Act4Screen* act4;

    int state;
    int selectedMission;

    unsigned int campaignSeed;
    unsigned int campaignCounter;
    sf::Clock campaignClock;

    void handleMenuEvent(const sf::Event& e);
    void handleCampaignEvent(const sf::Event& e);

    void startAct1(int mission);
    void startAct2();
    void startAct4(int mission);

    void returnToCampaign();

    void drawMenu();
    void drawCampaign();

public:
    Game();
    ~Game();

    void run();
};