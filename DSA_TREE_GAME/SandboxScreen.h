#pragma once

#include "Screen.h"
#include "MaxHeap.h"
#include "Events.h"
#include <string>

class SandboxScreen : public Screen
{
private:
    static const int MAX_VALUES = 32;

    sf::Font font;

    MaxHeap heap;
    EventLog log;

    int selectedFirst;
    int selectedSecond;

    int extracted[MAX_VALUES];
    int extractedCount;

    std::string input;
    std::string feedback;

    // 0 = no input mode, 1 = build array, 2 = insert value
    int inputMode;

    bool exitFlag;

    void resetLab();
    void buildHeap();
    void insertValue();
    void extractMaximum();
    void siftUpSelected();
    void siftDownSelected();
    void swapSelected();
    void heapSort();

    bool parseInput(int* values, int& count) const;
    bool parseSingleValue(int& value) const;

    void drawText(
        sf::RenderWindow& w,
        const std::string& str,
        float x,
        float y,
        unsigned int size,
        sf::Color color = sf::Color::White);

    void drawLine(
        sf::RenderWindow& w,
        sf::Vector2f start,
        sf::Vector2f end,
        sf::Color color);

    void drawHeapTree(sf::RenderWindow& w);
    void drawArray(sf::RenderWindow& w);
    void drawEventLog(sf::RenderWindow& w);
    void drawOperationPanel(sf::RenderWindow& w);

    void drawButton(
        sf::RenderWindow& w,
        const std::string& label,
        float x,
        float y,
        bool enabled);

    bool buttonHit(
        float mouseX,
        float mouseY,
        float x,
        float y) const;

    sf::Vector2f getNodePosition(int index) const;

    std::string eventName(int type) const;

    void handleButton(float mouseX, float mouseY);
    void handleTextInput(const sf::Event::TextEntered& textEvent);

public:
    explicit SandboxScreen(sf::RenderWindow& gameWindow);

    void handleEvent(const sf::Event& e) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& w) override;

    bool exitRequested() const;
};