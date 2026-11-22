#include "Game.h"
#include"SeededRandom.h"
#include <iostream>
#include <string>

Game::Game()
    : window(sf::VideoMode({ 1920, 1080 }), "Vault Heist"),
    activeScreen(nullptr),
    act1(nullptr),
    act2(nullptr),
    act4(nullptr)
{
    window.setFramerateLimit(60);

    if (!font.openFromFile("arial.ttf"))
        std::cout << "Failed to load game font\n";
        campaignSeed = 1;
        campaignCounter = 0;
    state = MENU;
    selectedMission = 0;
}

Game::~Game()
{
    delete activeScreen;
    activeScreen = nullptr;

    act1 = nullptr;
    act2 = nullptr;
    act4 = nullptr;
}

void Game::startAct1(int mission)
{
    delete activeScreen;

    act2 = nullptr;
    act4 = nullptr;

    unsigned int levelSeed = SeededRandom::deriveSeed(
        campaignSeed,
        1,
        static_cast<unsigned int>(mission)
    );

    act1 = new Act1Screen(window, mission, levelSeed);


    activeScreen = act1;

    selectedMission = mission;
    state = PLAYING;
}

void Game::startAct2()
{
    delete activeScreen;

    act1 = nullptr;
    act4 = nullptr;

    act2 = new Act2Screen(font, progress);
    activeScreen = act2;

    state = PLAYING;
}

void Game::startAct4(int mission)
{
    delete activeScreen;

    act1 = nullptr;
    act2 = nullptr;

    unsigned int levelSeed = SeededRandom::deriveSeed(
        campaignSeed,
        4,
        static_cast<unsigned int>(mission)
    );

    act4 = new Act4Screen(window, mission, levelSeed);

    activeScreen = act4;

    selectedMission = mission;
    state = PLAYING;
}

void Game::returnToCampaign()
{
    delete activeScreen;

    activeScreen = nullptr;

    act1 = nullptr;
    act2 = nullptr;
    act4 = nullptr;

    state = CAMPAIGN;
}

void Game::handleMenuEvent(const sf::Event& e)
{
    if (const auto* key = e.getIf<sf::Event::KeyPressed>())
    {
        if (key->code == sf::Keyboard::Key::Num1){
            
            campaignCounter++;

            unsigned int clockValue =
                static_cast<unsigned int>(
                    campaignClock.getElapsedTime().asMicroseconds()
                    );

            unsigned int freshSeed =
                clockValue ^
                (campaignCounter * 0x9E3779B9);

            campaignSeed = SeededRandom::deriveSeed(
                freshSeed,
                campaignCounter,
                clockValue >> 3
            );
            
            
            
            
            
            
            state = CAMPAIGN;
            }
        else if (key->code == sf::Keyboard::Key::Num2)
            state = SANDBOX;

        else if (key->code == sf::Keyboard::Key::Escape ||
            key->code == sf::Keyboard::Key::Num3)
            window.close();
    }
}

void Game::handleCampaignEvent(const sf::Event& e)
{
    if (const auto* key = e.getIf<sf::Event::KeyPressed>())
    {
        switch (key->code)
        {
        case sf::Keyboard::Key::Num1:
            startAct1(Missions::MISSION_1_1);
            break;

        case sf::Keyboard::Key::Num2:
            startAct1(Missions::MISSION_1_2);
            break;

        case sf::Keyboard::Key::Num3:
            startAct1(Missions::MISSION_1_3);
            break;

        case sf::Keyboard::Key::Num4:
            startAct1(Missions::MISSION_1_4);
            break;

        case sf::Keyboard::Key::Num5:
            startAct1(Missions::MISSION_1_5);
            break;

        case sf::Keyboard::Key::Num6:
            startAct2();
            break;

        case sf::Keyboard::Key::F1:
            startAct4(1);
            break;

        case sf::Keyboard::Key::F2:
            startAct4(2);
            break;

        case sf::Keyboard::Key::F3:
            startAct4(3);
            break;

        case sf::Keyboard::Key::F4:
            startAct4(4);
            break;

        case sf::Keyboard::Key::F5:
            startAct4(5);
            break;

        case sf::Keyboard::Key::Escape:
            state = MENU;
            break;

        default:
            break;
        }
    }
}

void Game::drawMenu()
{
    window.setView(window.getDefaultView());

    sf::Text title(font);
    title.setString("VAULT HEIST");
    title.setCharacterSize(90);
    title.setFillColor(sf::Color::White);
    title.setPosition({ 650.f, 200.f });

    sf::Text options(font);
    options.setString(
        "1. Campaign\n\n"
        "2. Sandbox (Coming Soon)\n\n"
        "3. Exit\n\n"
        "Press the corresponding number"
    );

    options.setCharacterSize(38);
    options.setFillColor(sf::Color::White);
    options.setPosition({ 700.f, 450.f });

    window.draw(title);
    window.draw(options);
}

void Game::drawCampaign()
{
    window.setView(window.getDefaultView());

    sf::Text title(font);
    title.setString("CAMPAIGN MAP");
    title.setCharacterSize(75);
    title.setFillColor(sf::Color::White);
    title.setPosition({ 650.f, 100.f });

    std::string campaignText =
        "ACT 1 - THE BLUEPRINT WING\n";

    for (int i = 0; i < 5; i++)
    {
        campaignText += std::to_string(i + 1);
        campaignText += ". ";

        switch (i)
        {
        case 0: campaignText += "Traversal Mission"; break;
        case 1: campaignText += "Reconstruction Mission"; break;
        case 2: campaignText += "Height Mission"; break;
        case 3: campaignText += "LCA Mission"; break;
        case 4: campaignText += "Mirror Mission"; break;
        }

        campaignText += "   [";

        for (int star = 0; star < 3; star++)
        {
            campaignText +=
                star < progress.stars[1][i] ? "*" : "-";
        }

        campaignText += "]\n";
    }

    campaignText +=
        "\n"
        "ACT 2 - THE SORTED WING\n"
        "6. Enter Act 2\n\n"
        "ACT 4 - THE HEAP FORTRESS\n";

    for (int i = 0; i < 5; i++)
    {
        campaignText += "F";
        campaignText += std::to_string(i + 1);
        campaignText += ". ";

        switch (i)
        {
        case 0: campaignText += "Activate the Vault"; break;
        case 1: campaignText += "Repair the Security Locks"; break;
        case 2: campaignText += "Extract the Greatest Treasure"; break;
        case 3: campaignText += "Race Against the Guards"; break;
        case 4: campaignText += "Final Vault Boss"; break;
        }

        campaignText += "   [";

        for (int star = 0; star < 3; star++)
        {
            campaignText +=
                star < progress.stars[4][i] ? "*" : "-";
        }

        campaignText += "]\n";
    }

    campaignText +=
        "\n"
        "ESC. Back to Main Menu";

    sf::Text options(font);
    options.setString(campaignText);
    options.setCharacterSize(25);
    options.setFillColor(sf::Color::White);
    options.setPosition({ 600.f, 240.f });

    window.draw(title);
    window.draw(options);
}

void Game::run()
{
    sf::Clock clock;

    while (window.isOpen())
    {
        while (const auto event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
                continue;
            }

            if (state == MENU)
            {
                handleMenuEvent(*event);
            }
            else if (state == CAMPAIGN)
            {
                handleCampaignEvent(*event);
            }
            else if (state == PLAYING && activeScreen != nullptr)
            {
                activeScreen->handleEvent(*event);
            }
            else if (state == SANDBOX)
            {
                if (const auto* key =
                    event->getIf<sf::Event::KeyPressed>())
                {
                    if (key->code == sf::Keyboard::Key::Escape)
                        state = MENU;
                }
            }
        }

        float dt = clock.restart().asSeconds();

        window.clear(sf::Color(15, 18, 30));

        if (state == MENU)
        {
            drawMenu();
        }
        else if (state == CAMPAIGN)
        {
            drawCampaign();
        }
        else if (state == PLAYING && activeScreen != nullptr)
        {
            activeScreen->update(dt);
            activeScreen->draw(window);

            if (act1 != nullptr)
            {
                if (act1->retryRequested())
                {
                    int missionToRetry = selectedMission;
                    startAct1(missionToRetry);
                }
                else if (act1->exitRequested())
                {
                    if (act1->isMissionDone() &&
                        !act1->getAlarm().isFailed())
                    {
                        int missionIndex = selectedMission - 1;

                        if (missionIndex >= 0 && missionIndex < 5)
                        {
                            int stars = act1->getAlarm().getStars();

                            if (stars > progress.stars[1][missionIndex])
                                progress.stars[1][missionIndex] = stars;
                        }
                    }

                    returnToCampaign();
                }
            }
            else if (act2 != nullptr)
            {
                if (act2->exitRequested())
                {
                    act2->clearExit();
                    returnToCampaign();
                }
            }
            else if (act4 != nullptr)
            {
                if (act4->retryRequested())
                {
                    int missionToRetry = selectedMission;
                    startAct4(missionToRetry);
                }
                else if (act4->exitRequested())
                {
                    if (act4->isMissionDone() &&
                        !act4->getAlarm().isFailed())
                    {
                        int missionIndex = selectedMission - 1;

                        if (missionIndex >= 0 && missionIndex < 5)
                        {
                            int stars = act4->getAlarm().getStars();

                            if (stars > progress.stars[4][missionIndex])
                                progress.stars[4][missionIndex] = stars;
                        }
                    }

                    returnToCampaign();
                }
            }
        }
        else if (state == SANDBOX)
        {
            sf::Text message(font);

            message.setString(
                "SANDBOX\n\nComing Soon\n\nPress ESC to return"
            );

            message.setCharacterSize(55);
            message.setFillColor(sf::Color::White);
            message.setPosition({ 650.f, 400.f });

            window.draw(message);
        }

        window.display();
    }
}