#pragma once

#include "Screen.h"
#include "MaxHeap.h"
#include "Events.h"
#include "AlarmSystem.h"
#include <string>

class Act4Screen : public Screen {
private:
    static const int MAX_HISTORY = 64;
    static const int MAX_EVENTS = 512;

    struct Snapshot {
        int heap[32];
        int heapCount;

        int extracted[32];
        int extractedCount;

        int operations;
        int alarmSteps;

        float timer;

        bool missionDone;
        bool missionFailed;

        int eventCount;
    };

    sf::Font font;

    MaxHeap heap;
    EventLog log;
    AlarmSystem alarm;

    int mission;
    int totalCount;
    int optimalSteps;

    int initialValues[32];

    int selectedFirst;
    int selectedSecond;

    int extracted[32];
    int extractedCount;

    int operations;

    float timer;
    float timeLimit;
    int operationLimit;

    bool exitFlag;
    bool retryFlag;
    bool missionDone;
    bool missionFailed;

    std::string feedback;

    // Undo and replay
    Snapshot history[MAX_HISTORY];
    Events eventHistory[MAX_EVENTS];

    int historyCount;
    int historyIndex;

    bool replaying;
    float replayTimer;
    float replayDelay;

    // Swap animation
    bool animatingSwap;
    float swapTimer;
    float swapDuration;

    int animValueA;
    int animValueB;

    int animIndexA;
    int animIndexB;
    int replayIndex;

    sf::Vector2f animStartA;
    sf::Vector2f animStartB;

    // Comparison highlighting
    int compareA;
    int compareB;

    unsigned int levelSeed;

    void resetMission();

    void checkCompletion();
    void performExtraction();
    void performSwap();

    void consumeAction();

    void saveHistory();
    void restoreSnapshot(const Snapshot& snapshot);

    void undo();
    void startReplay();

    void drawHUD(sf::RenderWindow& w);
    void drawHeapTree(sf::RenderWindow& w);
    void drawArray(sf::RenderWindow& w);
    void drawEventLog(sf::RenderWindow& w);
    void drawOperationPanel(sf::RenderWindow& w);
    void drawResultOverlay(sf::RenderWindow& w);

    void drawText(sf::RenderWindow& w,
        const std::string& str,
        float x, float y,
        unsigned int size,
        sf::Color color = sf::Color::White);

    void drawLine(sf::RenderWindow& w,
        sf::Vector2f start,
        sf::Vector2f end,
        sf::Color color);

    void drawButton(sf::RenderWindow& w,
        const std::string& label,
        float x, float y,
        bool enabled);

    bool buttonHit(float mouseX, float mouseY,
        float x, float y) const;

    void handleButton(float mouseX, float mouseY);

    sf::Vector2f getNodePosition(int index) const;

    std::string eventName(int type) const;

public:
    Act4Screen(
        sf::RenderWindow& gameWindow,
        int selectedMission = 1,
        unsigned int seed = 1);

    void handleEvent(const sf::Event& e) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& w) override;

    bool exitRequested() const;
    bool retryRequested() const;
    bool isMissionDone() const;

    const AlarmSystem& getAlarm() const;
};