#pragma once

#include <SFML/Graphics.hpp>

#include "Screen.h"
#include "Progress.h"
#include "Act1Screen.h"
#include "Act2Screen.h"
#include "Act3Screen.h"
#include "Act4Screen.h"
#include "SandboxScreen.h"
#include "TreeSandbox.h"
#include "SaveManager.h"

class Game {
private:
    static const int MENU = 0;
    static const int CAMPAIGN = 1;
    static const int PLAYING = 2;
    static const int SANDBOX = 3;
    static const int TREE_SANDBOX = 4;

    bool campaignActive;

    static constexpr const char* SAVE_PATH = "savegame.sav";

    sf::RenderWindow window;
    sf::Font font;

    Progress progress;

    Screen* activeScreen;

    Act1Screen* act1;
    Act2Screen* act2;
    Act3Screen* act3;
    Act4Screen* act4;

    TreeSandbox* treeSandbox;

    int state;
    int selectedMission;

    unsigned int campaignSeed;
    unsigned int campaignCounter;
    sf::Clock campaignClock;

    char menuMessage[160];

    void handleMenuEvent(const sf::Event& e);
    void handleCampaignEvent(const sf::Event& e);

    void startNewCampaign();
    void loadCampaign();
    void saveCampaign();
    void setMenuMessage(const char* message);

    void startAct1(int mission);
    void startAct2();
    void startAct3();
    void startAct4(int mission);

    void startSandbox();
    void startTreeSandbox();

    void returnToCampaign();
    void returnToMenu();

    void drawMenu();
    void drawCampaign();

public:
    Game();
    ~Game();

    void run();
};