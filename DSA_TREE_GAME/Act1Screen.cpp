#include "Act1Screen.h"
#include "Missions.h"
#include"Act1LevelGenerator.h"
#include <iostream>
#include <string>
#include <cmath>


Act1Screen::Act1Screen(sf::RenderWindow& gameWindow, int mission, unsigned int seed)
{
levelSeed = seed;
    window = &gameWindow;

    tree = new BinaryTree();
    log = new EventLog();

    comparisonRoot = createComparisonTree(levelSeed);

    tree->setTree(createSampleTree(levelSeed));

    visualizer = new Act1(tree,levelSeed);
    visualizer->setEventLog(log);

    exitFlag = false;
    retryFlag = false;
    missionDone = false;
    syncedActionCount = 0;

    uiFontLoaded = uiFont.openFromFile("arial.ttf");

    if (!uiFontLoaded)
        std::cout << "Failed to load alarm HUD font: arial.ttf\n";

    if (mission != 0)
    {
        visualizer->resetMissionActionCount();
        visualizer->setMission(mission);
        visualizer->startMission();

        int optimalSteps = visualizer->getOptimalMissionSteps();

        alarm.start(optimalSteps);

      
    }
}

Act1Screen::~Act1Screen()
{
    delete visualizer;
    visualizer = nullptr;

    delete log;
    log = nullptr;

    delete tree;
    tree = nullptr;

    delete comparisonRoot;
    comparisonRoot = nullptr;
}

TreeNode* Act1Screen::createSampleTree(unsigned int seed)
{
    return Act1LevelGenerator::generate(
        SeededRandom::deriveSeed(seed, 1, 1)
    );
}

TreeNode* Act1Screen::createComparisonTree(unsigned int seed)
{
    return Act1LevelGenerator::generate(
        SeededRandom::deriveSeed(seed, 1, 2)
    );
}

void Act1Screen::syncAlarmActions()
{
    if (visualizer == nullptr ||
        alarm.isFailed() ||
        alarm.isCompleted())
        return;

    int currentActions = visualizer->getMissionActionCount();
    int difference = currentActions - syncedActionCount;

    if (difference > 0)
    {
        alarm.addSteps(difference);
        syncedActionCount = currentActions;
    }
}

void Act1Screen::handleEvent(const sf::Event& e)
{
    if (visualizer == nullptr || window == nullptr)
        return;

    // Once a result is available, gameplay is over.
    if (missionDone || alarm.isFailed())
    {
        if (const auto* key = e.getIf<sf::Event::KeyPressed>())
        {
            if (key->code == sf::Keyboard::Key::Enter ||
                key->code == sf::Keyboard::Key::Escape)
            {
                exitFlag = true;
            }
            else if (key->code == sf::Keyboard::Key::R)
            {
                retryFlag = true;
            }
        }

        return;
    }

    if (const auto* key = e.getIf<sf::Event::KeyPressed>())
    {
        bool active = visualizer->isMissionActive();

        if (key->code == sf::Keyboard::Key::Escape)
        {
            exitFlag = true;
            return;
        }

        if (!active)
        {
            if (key->code == sf::Keyboard::Key::Space)
                visualizer->togglePlayback();

            if (key->code == sf::Keyboard::Key::R)
                visualizer->restart();

            if (key->code == sf::Keyboard::Key::Right)
                visualizer->nextEvent();

            if (key->code == sf::Keyboard::Key::Left)
                visualizer->previousEvent();

            if (key->code == sf::Keyboard::Key::Num1)
                visualizer->runTraversal(1);

            if (key->code == sf::Keyboard::Key::Num2)
                visualizer->runTraversal(2);

            if (key->code == sf::Keyboard::Key::Num3)
                visualizer->runTraversal(3);

            if (key->code == sf::Keyboard::Key::Num4)
                visualizer->runTraversal(4);

            if (key->code == sf::Keyboard::Key::Num5)
                visualizer->runTraversal(5);

            if (key->code == sf::Keyboard::Key::Num6)
                visualizer->runTraversal(6);

            if (key->code == sf::Keyboard::Key::Num7)
                visualizer->runTraversal(7);

            if (key->code == sf::Keyboard::Key::Num8)
                visualizer->runTraversal(8);
        }

        if (key->code == sf::Keyboard::Key::C)
            visualizer->resetCamera();

        if (key->code == sf::Keyboard::Key::M)
        {
            if (active &&
                visualizer->getMission() == Missions::MISSION_1_5)
            {
                visualizer->swapMirrorNode();
            }
            else if (!active)
            {
                visualizer->mirrorTree();
            }
        }

        if (key->code == sf::Keyboard::Key::P)
            visualizer->showPathToSelectedNode();

        if (key->code == sf::Keyboard::Key::L)
            visualizer->toggleLCAMode();

        if (key->code == sf::Keyboard::Key::S)
            visualizer->checkSymmetry();

        if (key->code == sf::Keyboard::Key::T)
            visualizer->compareTree(comparisonRoot);

        visualizer->handleInputKey(key->code);

        if (key->code == sf::Keyboard::Key::N && !active)
            visualizer->createNewTree();

        if (key->code == sf::Keyboard::Key::R && !active)
            visualizer->startRootInput();

        if (key->code == sf::Keyboard::Key::A)
        {
            if (active &&
                visualizer->getMission() == Missions::MISSION_1_5)
            {
                alarm.useHint();
                visualizer->autoMirror();
            }
            else if (!active)
            {
                visualizer->startChildInput();
            }
        }

        if (key->code == sf::Keyboard::Key::Enter)
        {
            if (active)
            {
                if (visualizer->getMission() == Missions::MISSION_1_1)
                {
                    visualizer->submitTraversalAnswer();
                }
                else if (visualizer->getMission() == Missions::MISSION_1_2)
                {
                    visualizer->submitReconstruction();
                }
                else if (visualizer->getMission() == Missions::MISSION_1_3)
                {
                    visualizer->submitHeightInput();
                }
                else if (visualizer->getMission() == Missions::MISSION_1_4)
                {
                    if (!visualizer->isLCAPhaseComplete())
                        visualizer->submitLCAAnswer();
                    else
                        visualizer->submitPlayerPath();
                }
                else if (visualizer->getMission() == Missions::MISSION_1_5)
                {
                    visualizer->submitMirrorAnswer();
                }
            }
            else
            {
                visualizer->findLCA();
            }
        }
    }

    if (const auto* mouse = e.getIf<sf::Event::MouseButtonReleased>())
    {
        if (mouse->button == sf::Mouse::Button::Left)
        {
            sf::Vector2f mousePosition(
                static_cast<float>(mouse->position.x),
                static_cast<float>(mouse->position.y)
            );

            if (visualizer->isMissionActive() &&
                visualizer->getMission() == Missions::MISSION_1_2)
            {
                visualizer->dropToken(mousePosition, *window);
            }
            else
            {
                visualizer->stopDragging();
            }
        }
    }

    if (const auto* mouse = e.getIf<sf::Event::MouseMoved>())
    {
        sf::Vector2f mousePosition(
            static_cast<float>(mouse->position.x),
            static_cast<float>(mouse->position.y)
        );

        if (visualizer->isMissionActive() &&
            visualizer->getMission() == Missions::MISSION_1_2)
        {
            visualizer->updateTokenDrag(mousePosition);
        }
        else
        {
            visualizer->dragView(mousePosition);
        }
    }

    if (const auto* mouse = e.getIf<sf::Event::MouseWheelScrolled>())
    {
        visualizer->zoom(mouse->delta);
    }

    if (const auto* mouse = e.getIf<sf::Event::MouseButtonPressed>())
    {
        if (mouse->button == sf::Mouse::Button::Left)
        {
            sf::Vector2f mousePosition(
                static_cast<float>(mouse->position.x),
                static_cast<float>(mouse->position.y)
            );

            if (visualizer->isMissionActive() &&
                visualizer->getMission() == Missions::MISSION_1_2)
            {
                visualizer->startTokenDrag(mousePosition);
            }
            else
            {
                visualizer->startDragging(mousePosition);
                visualizer->selectNode(mousePosition, *window);
            }
        }
    }

    if (const auto* text = e.getIf<sf::Event::TextEntered>())
    {
        if (visualizer->isMissionActive() &&
            visualizer->getMission() == Missions::MISSION_1_3)
        {
            char character = static_cast<char>(text->unicode);
            visualizer->handleHeightTextInput(character);
        }
        else
        {
            visualizer->handleTextInput(text->unicode);
        }
    }

    syncAlarmActions();
}

void Act1Screen::update(float dt)
{
    if (visualizer == nullptr)
        return;

    if (!missionDone && !alarm.isFailed())
    {
        visualizer->update(dt);
        syncAlarmActions();

        if (visualizer->isMissionComplete())
        {
            alarm.complete();
            missionDone = true;
        }
    }
}

void Act1Screen::draw(sf::RenderWindow& w)
{
    if (visualizer != nullptr)
        visualizer->draw(w);

    drawAlarmHUD(w);

    if (missionDone || alarm.isFailed())
        drawResultOverlay(w);
}

void Act1Screen::drawAlarmHUD(sf::RenderWindow& w)
{
    if (!uiFontLoaded || alarm.getAlarmCapacity() <= 0)
        return;

    sf::View previousView = w.getView();
    w.setView(w.getDefaultView());

    const float panelWidth = 360.f;
    const float panelHeight = 190.f;
    const float margin = 24.f;

    sf::Vector2u windowSize = w.getSize();

    float panelX = static_cast<float>(windowSize.x) -
        panelWidth - margin;

    float panelY = margin;

    sf::RectangleShape panel(
        sf::Vector2f(panelWidth, panelHeight)
    );

    panel.setPosition(sf::Vector2f(panelX, panelY));
    panel.setFillColor(sf::Color(18, 23, 35, 235));
    panel.setOutlineThickness(2.f);
    panel.setOutlineColor(sf::Color(75, 85, 105));

    w.draw(panel);

    auto drawText = [&](const std::string& value,
        float x, float y, unsigned int size, sf::Color color)
        {
            sf::Text text(uiFont, value, size);
            text.setPosition(sf::Vector2f(x, y));
            text.setFillColor(color);
            w.draw(text);
        };

    const int current = alarm.getAlarmSteps();
    const int capacity = alarm.getAlarmCapacity();
    const int remaining = alarm.getRemainingAlarm();

    float ratio = 0.f;

    if (capacity > 0)
        ratio = static_cast<float>(current) / capacity;

    if(ratio < 0.f) ratio = 0.f;
    if(ratio > 1.f) ratio = 1.f;

    sf::Color barColor(50, 190, 110);

    if (ratio >= 0.75f)
        barColor = sf::Color(220, 65, 65);
    else if (ratio >= 0.50f)
        barColor = sf::Color(235, 155, 45);

    drawText("VAULT ALARM", panelX + 20.f, panelY + 12.f,
        23, sf::Color::White);

    drawText(
        std::to_string(current) + " / " +
        std::to_string(capacity) + " STEPS",
        panelX + 20.f, panelY + 48.f,
        19, sf::Color::White
    );

    sf::RectangleShape barBackground(
        sf::Vector2f(panelWidth - 40.f, 17.f)
    );

    barBackground.setPosition(
        sf::Vector2f(panelX + 20.f, panelY + 82.f)
    );

    barBackground.setFillColor(sf::Color(55, 62, 75));
    w.draw(barBackground);

    sf::RectangleShape barFill(
        sf::Vector2f((panelWidth - 40.f) * ratio, 17.f)
    );

    barFill.setPosition(
        sf::Vector2f(panelX + 20.f, panelY + 82.f)
    );

    barFill.setFillColor(barColor);
    w.draw(barFill);

    drawText(
        std::to_string(remaining) + " STEPS REMAINING",
        panelX + 20.f, panelY + 106.f,
        15, sf::Color(190, 200, 215)
    );

    drawText(
        "SCORE: " + std::to_string(alarm.getScore()),
        panelX + 20.f, panelY + 137.f,
        17, sf::Color(245, 215, 120)
    );

    drawText(
        "PENALTIES: " + std::to_string(alarm.getPenaltyCount()),
        panelX + 195.f, panelY + 137.f,
        15, sf::Color(220, 220, 220)
    );

    w.setView(previousView);
}

void Act1Screen::drawResultOverlay(sf::RenderWindow& w)
{
    if (!uiFontLoaded)
        return;

    sf::View previousView = w.getView();
    w.setView(w.getDefaultView());

    sf::Vector2u size = w.getSize();

    float centerX = static_cast<float>(size.x) / 2.f;
    float centerY = static_cast<float>(size.y) / 2.f;

    sf::RectangleShape shade(
        sf::Vector2f(
            static_cast<float>(size.x),
            static_cast<float>(size.y)
        )
    );

    shade.setFillColor(sf::Color(0, 0, 0, 175));
    w.draw(shade);

    sf::RectangleShape panel(sf::Vector2f(650.f, 440.f));
    panel.setPosition(sf::Vector2f(centerX - 325.f, centerY - 220.f));
    panel.setFillColor(sf::Color(18, 23, 35, 250));
    panel.setOutlineThickness(3.f);

    bool success = missionDone && alarm.isCompleted();

    panel.setOutlineColor(
        success ? sf::Color(65, 190, 115) : sf::Color(220, 65, 65)
    );

    w.draw(panel);

    auto drawText = [&](const std::string& value,
        float x, float y, unsigned int characterSize, sf::Color color)
        {
            sf::Text text(uiFont, value, characterSize);
            text.setFillColor(color);
            text.setPosition(sf::Vector2f(x, y));
            w.draw(text);
        };

    if (success)
        drawText("MISSION COMPLETE!", centerX - 190.f,
            centerY - 175.f, 38, sf::Color(90, 230, 145));
    else
        drawText("VAULT ALARM TRIGGERED", centerX - 230.f,
            centerY - 175.f, 34, sf::Color(255, 90, 90));

    drawText(
        "SCORE: " + std::to_string(alarm.getScore()),
        centerX - 130.f, centerY - 95.f,
        26, sf::Color::White
    );

    drawText(
        "STARS: " + std::to_string(alarm.getStars()) + " / 3",
        centerX - 130.f, centerY - 45.f,
        26, sf::Color(245, 215, 120)
    );

    drawText(
        "ALARM STEPS: " + std::to_string(alarm.getAlarmSteps()) +
        " / " + std::to_string(alarm.getAlarmCapacity()),
        centerX - 180.f, centerY + 5.f,
        20, sf::Color(210, 220, 235)
    );

    drawText(
        "PENALTIES: " + std::to_string(alarm.getPenaltyCount()),
        centerX - 130.f, centerY + 45.f,
        20, sf::Color(210, 220, 235)
    );

    drawText(
        "ENTER / ESC  -  CAMPAIGN",
        centerX - 180.f, centerY + 115.f,
        18, sf::Color::White
    );

    drawText(
        "R  -  RETRY MISSION",
        centerX - 150.f, centerY + 150.f,
        18, sf::Color(130, 200, 255)
    );

    w.setView(previousView);
}

bool Act1Screen::exitRequested() const
{
    return exitFlag;
}

bool Act1Screen::retryRequested() const
{
    return retryFlag;
}

bool Act1Screen::isMissionDone() const
{
    return missionDone;
}

void Act1Screen::clearExit()
{
    exitFlag = false;
}

AlarmSystem& Act1Screen::getAlarm()
{
    return alarm;
}

const AlarmSystem& Act1Screen::getAlarm() const
{
    return alarm;
}