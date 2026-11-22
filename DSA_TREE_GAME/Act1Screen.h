#pragma once

#include "Screen.h"
#include "Act1.h"
#include "BinaryTree.h"
#include "Events.h"
#include "AlarmSystem.h"

class Act1Screen : public Screen
{
private:
    sf::RenderWindow* window;

    BinaryTree* tree;
    EventLog* log;
    Act1* visualizer;

    TreeNode* comparisonRoot;

    AlarmSystem alarm;

    sf::Font uiFont;
    bool uiFontLoaded;

    bool exitFlag;
    bool retryFlag;
    bool missionDone;

    int syncedActionCount;

    unsigned int levelSeed;

    TreeNode* createSampleTree(unsigned int seed);
    TreeNode* createComparisonTree(unsigned int seed);

    void syncAlarmActions();
    void drawAlarmHUD(sf::RenderWindow& w);
    void drawResultOverlay(sf::RenderWindow& w);

public:
    Act1Screen(sf::RenderWindow& gameWindow, int mission = 0, unsigned int seed = 1);
    ~Act1Screen();

    void handleEvent(const sf::Event& e) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& w) override;

    bool exitRequested() const;
    bool retryRequested() const;
    bool isMissionDone() const;
    void clearExit();

    AlarmSystem& getAlarm();
    const AlarmSystem& getAlarm() const;
};