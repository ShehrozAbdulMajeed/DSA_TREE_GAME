#include "SandboxScreen.h"
#include <cmath>

SandboxScreen::SandboxScreen(sf::RenderWindow& gameWindow)
{
    (void)gameWindow;

    font.openFromFile("arial.ttf");

    selectedFirst = -1;
    selectedSecond = -1;

    extractedCount = 0;

    inputMode = 0;
    exitFlag = false;

    input.clear();
    feedback = "Enter values, then build a Max Heap.";

    resetLab();
}

void SandboxScreen::resetLab()
{
    int empty[1] = {};

    heap.setRawArray(empty, 0);
    log.clear();

    selectedFirst = -1;
    selectedSecond = -1;

    extractedCount = 0;

    input.clear();
    inputMode = 0;

    feedback = "Sandbox reset. Enter values to begin.";
}

bool SandboxScreen::parseInput(int* values, int& count) const
{
    count = 0;

    int i = 0;
    int length = static_cast<int>(input.length());

    while (i < length)
    {
        while (i < length &&
            (input[i] == ' ' ||
                input[i] == ',' ||
                input[i] == '\t'))
        {
            i++;
        }

        if (i >= length)
            break;

        int sign = 1;

        if (input[i] == '-')
        {
            sign = -1;
            i++;
        }
        else if (input[i] == '+')
        {
            i++;
        }

        if (i >= length ||
            input[i] < '0' ||
            input[i] > '9')
            return false;

        int number = 0;

        while (i < length &&
            input[i] >= '0' &&
            input[i] <= '9')
        {
            number = number * 10 + (input[i] - '0');
            i++;
        }

        if (count >= MAX_VALUES)
            return false;

        values[count++] = number * sign;

        if (i < length&&
            input[i] != ' ' &&
            input[i] != ',' &&
            input[i] != '\t')
        {
            return false;
        }
    }

    return count > 0;
}

bool SandboxScreen::parseSingleValue(int& value) const
{
    int values[MAX_VALUES] = {};
    int count = 0;

    if (!parseInput(values, count))
        return false;

    if (count != 1)
        return false;

    value = values[0];
    return true;
}

void SandboxScreen::buildHeap()
{
    int values[MAX_VALUES] = {};
    int count = 0;

    if (!parseInput(values, count))
    {
        feedback =
            "Invalid input. Use numbers separated by spaces or commas.";

        return;
    }

    heap.buildMaxHeap(values, count, &log);

    selectedFirst = -1;
    selectedSecond = -1;

    extractedCount = 0;

    inputMode = 0;

    feedback =
        "Max Heap built successfully. Select an array element to operate on.";
}

void SandboxScreen::insertValue()
{
    int value = 0;

    if (!parseSingleValue(value))
    {
        feedback = "Enter exactly one valid integer to insert.";
        return;
    }

    if (heap.getCount() >= MAX_VALUES)
    {
        feedback = "Heap is full. Maximum capacity is 32 elements.";
        return;
    }

    heap.insert(value, &log);

    selectedFirst = -1;
    selectedSecond = -1;

    inputMode = 0;

    feedback = "Value inserted into the Max Heap.";
}

void SandboxScreen::extractMaximum()
{
    if (heap.getCount() <= 0)
    {
        feedback = "The heap is empty.";
        return;
    }

    if (extractedCount >= MAX_VALUES)
    {
        feedback = "Sorted output is full. Reset the sandbox to continue.";
        return;
    }

    int maximum = heap.extractMax(&log);

    extracted[extractedCount++] = maximum;

    selectedFirst = -1;
    selectedSecond = -1;

    feedback =
        "Extracted maximum: " + std::to_string(maximum);
}

void SandboxScreen::siftUpSelected()
{
    if (selectedFirst < 0 ||
        selectedFirst >= heap.getCount())
    {
        feedback = "Select an active heap element first.";
        return;
    }

    heap.siftUp(selectedFirst, &log);

    selectedFirst = -1;
    selectedSecond = -1;

    feedback = "Sift Up operation completed.";
}

void SandboxScreen::siftDownSelected()
{
    if (selectedFirst < 0 ||
        selectedFirst >= heap.getCount())
    {
        feedback = "Select an active heap element first.";
        return;
    }

    heap.siftDown(
        selectedFirst,
        heap.getCount(),
        &log);

    selectedFirst = -1;
    selectedSecond = -1;

    feedback = "Sift Down operation completed.";
}

void SandboxScreen::swapSelected()
{
    if (selectedFirst < 0 ||
        selectedSecond < 0 ||
        selectedFirst == selectedSecond)
    {
        feedback = "Select two different array elements first.";
        return;
    }

    if (!heap.swapElements(
        selectedFirst,
        selectedSecond,
        &log))
    {
        feedback = "The selected elements could not be swapped.";
        return;
    }

    selectedFirst = -1;
    selectedSecond = -1;

    feedback = "Elements swapped. Check whether the heap property still holds.";
}

void SandboxScreen::heapSort()
{
    if (heap.getCount() <= 0)
    {
        feedback = "There are no elements to sort.";
        return;
    }
    extractedCount = 0;
    while (heap.getCount() > 0 &&
        extractedCount < MAX_VALUES)
    {
        int maximum = heap.extractMax(&log);
        extracted[extractedCount++] = maximum;
    }

    log.add(EventTypes::HEAP_SORT_COMPLETE);

    selectedFirst = -1;
    selectedSecond = -1;

    feedback = "Heap Sort completed. The sorted output is shown below.";
}

void SandboxScreen::drawText(
    sf::RenderWindow& w,
    const std::string& str,
    float x,
    float y,
    unsigned int size,
    sf::Color color)
{
    sf::Text text(font);

    text.setString(str);
    text.setCharacterSize(size);
    text.setFillColor(color);
    text.setPosition({ x, y });

    w.draw(text);
}

sf::Vector2f SandboxScreen::getNodePosition(int index) const
{
    int depth = 0;
    int firstIndex = 0;
    int nodesOnLevel = 1;

    while (index >= firstIndex + nodesOnLevel)
    {
        firstIndex += nodesOnLevel;
        nodesOnLevel *= 2;
        depth++;
    }

    int positionOnLevel = index - firstIndex;

    float x = 170.f +
        680.f *
        (static_cast<float>(positionOnLevel + 1) /
            static_cast<float>(nodesOnLevel + 1));

    float y = 310.f + depth * 125.f;

    return { x, y };
}

void SandboxScreen::drawLine(
    sf::RenderWindow& w,
    sf::Vector2f start,
    sf::Vector2f end,
    sf::Color color)
{
    float dx = end.x - start.x;
    float dy = end.y - start.y;

    float length = std::sqrt(dx * dx + dy * dy);

    float angle =
        std::atan2(dy, dx) * 180.f / 3.14159265f;

    sf::RectangleShape line({ length, 4.f });

    line.setPosition(start);
    line.setRotation(sf::degrees(angle));
    line.setFillColor(color);

    w.draw(line);
}

void SandboxScreen::drawHeapTree(sf::RenderWindow& w)
{
    int count = heap.getCount();

    int compareA = -1;
    int compareB = -1;

    for (int i = log.size() - 1; i >= 0; i--)
    {
        const Events& event = log.get(i);

        if (event.type == EventTypes::COMPARE)
        {
            compareA = event.id;
            compareB = event.extra;
            break;
        }
    }

    for (int i = 1; i < count; i++)
    {
        int parent = (i - 1) / 2;

        sf::Color edgeColor(105, 125, 155);

        if (heap.getValue(parent) < heap.getValue(i))
            edgeColor = sf::Color(235, 65, 65);

        if ((parent == compareA && i == compareB) ||
            (parent == compareB && i == compareA))
        {
            edgeColor = sf::Color(255, 205, 65);
        }

        drawLine(
            w,
            getNodePosition(parent),
            getNodePosition(i),
            edgeColor);
    }

    for (int i = 0; i < count; i++)
    {
        sf::Vector2f position = getNodePosition(i);

        sf::CircleShape node(32.f);

        node.setOrigin({ 32.f, 32.f });
        node.setPosition(position);

        if (i == selectedFirst || i == selectedSecond)
            node.setFillColor(sf::Color(235, 160, 45));

        else if (i == 0)
            node.setFillColor(sf::Color(40, 160, 95));

        else if (i == compareA || i == compareB)
            node.setFillColor(sf::Color(165, 105, 195));

        else
            node.setFillColor(sf::Color(55, 105, 175));

        node.setOutlineThickness(3.f);
        node.setOutlineColor(sf::Color::White);

        w.draw(node);

        drawText(
            w,
            std::to_string(heap.getValue(i)),
            position.x - 15.f,
            position.y - 16.f,
            24);
    }

    drawText(
        w,
        "ROOT = MAXIMUM",
        105.f, 280.f, 20,
        sf::Color(100, 230, 150));

    drawText(
        w,
        "RED EDGE = INVALID HEAP RELATIONSHIP",
        105.f, 325.f, 17,
        sf::Color(255, 105, 105));
}

void SandboxScreen::drawArray(sf::RenderWindow& w)
{
    float startX = 950.f;
    float boxY = 335.f;

    float boxWidth = 56.f;
    float boxHeight = 65.f;
    float gap = 6.f;

    drawText(
        w,
        "ACTIVE HEAP ARRAY",
        startX, 285.f, 23,
        sf::Color(110, 205, 255));

    int count = heap.getCount();

    for (int i = 0; i < count; i++)
    {
        float x = startX + i * (boxWidth + gap);

        sf::RectangleShape box({ boxWidth, boxHeight });

        box.setPosition({ x, boxY });

        if (i == selectedFirst || i == selectedSecond)
            box.setFillColor(sf::Color(235, 160, 45));

        else if (i == 0)
            box.setFillColor(sf::Color(40, 160, 95));

        else
            box.setFillColor(sf::Color(55, 105, 175));

        box.setOutlineThickness(2.f);
        box.setOutlineColor(sf::Color::White);

        w.draw(box);

        drawText(
            w,
            std::to_string(heap.getValue(i)),
            x + 12.f,
            boxY + 15.f,
            22);

        drawText(
            w,
            std::to_string(i),
            x + 22.f,
            boxY + 70.f,
            16,
            sf::Color(180, 190, 205));
    }

    float sortedY = 500.f;

    drawText(
        w,
        "EXTRACTED / SORTED REGION",
        startX, 450.f, 22,
        sf::Color(100, 230, 150));

    for (int i = 0; i < extractedCount; i++)
    {
        float x = startX + i * (boxWidth + gap);

        sf::RectangleShape box({ boxWidth, boxHeight });

        box.setPosition({ x, sortedY });
        box.setFillColor(sf::Color(35, 125, 85));
        box.setOutlineThickness(2.f);
        box.setOutlineColor(sf::Color::White);

        w.draw(box);

        int value = extracted[extractedCount - 1 - i];

        drawText(
            w,
            std::to_string(value),
            x + 12.f,
            sortedY + 15.f,
            22);
    }

    drawText(
        w,
        "Active elements: " + std::to_string(count),
        startX, 590.f, 18,
        sf::Color(180, 195, 215));

    drawText(
        w,
        "Sorted elements: " + std::to_string(extractedCount),
        startX, 620.f, 18,
        sf::Color(120, 230, 160));
}

std::string SandboxScreen::eventName(int type) const
{
    switch (type)
    {
    case EventTypes::VISIT: return "VISIT";
    case EventTypes::COMPARE: return "COMPARE";
    case EventTypes::INSERT: return "INSERT";
    case EventTypes::UNLINK: return "UNLINK";
    case EventTypes::SWAP_VALUE: return "SWAP VALUE";
    case EventTypes::ROTATE: return "ROTATE";
    case EventTypes::UPDATE_HEIGHT: return "UPDATE HEIGHT";
    case EventTypes::UPDATE_SIZE: return "UPDATE SIZE";
    case EventTypes::BALANCE_ALERT: return "BALANCE ALERT";
    case EventTypes::HIGHLIGHT_SUBTREE: return "HIGHLIGHT";
    case EventTypes::HEAP_BUILD: return "HEAP BUILD";
    case EventTypes::HEAP_SWAP: return "HEAP SWAP";
    case EventTypes::HEAPIFY_START: return "HEAPIFY START";
    case EventTypes::HEAPIFY_DONE: return "HEAPIFY DONE";
    case EventTypes::EXTRACT_MAX: return "EXTRACT MAX";
    case EventTypes::MARK_SORTED: return "MARK SORTED";
    case EventTypes::HEAP_SIZE_CHANGE: return "SIZE CHANGE";
    case EventTypes::HEAP_SORT_COMPLETE: return "HEAP SORT COMPLETE";
    case EventTypes::DONE: return "DONE";
    default: return "UNKNOWN";
    }
}

void SandboxScreen::drawEventLog(sf::RenderWindow& w)
{
    float x = 1430.f;
    float y = 275.f;

    sf::RectangleShape panel({ 440.f, 375.f });

    panel.setPosition({ x, y });
    panel.setFillColor(sf::Color(24, 30, 45));
    panel.setOutlineThickness(2.f);
    panel.setOutlineColor(sf::Color(95, 120, 155));

    w.draw(panel);

    drawText(
        w,
        "EVENT LOG",
        x + 18.f, y + 15.f, 25,
        sf::Color(110, 205, 255));

    int total = log.size();
    int start = total - 8;

    if (start < 0)
        start = 0;

    for (int i = start; i < total; i++)
    {
        const Events& event = log.get(i);

        std::string line =
            std::to_string(i + 1) + ". " +
            eventName(event.type);

        if (event.type == EventTypes::COMPARE ||
            event.type == EventTypes::HEAP_SWAP)
        {
            line += " [" +
                std::to_string(event.id) + ", " +
                std::to_string(event.extra) + "]";
        }
        else if (event.type == EventTypes::EXTRACT_MAX)
        {
            line += " [" +
                std::to_string(event.data) + "]";
        }
        else if (event.type == EventTypes::HEAP_SIZE_CHANGE)
        {
            line += " [" +
                std::to_string(event.data) + "]";
        }

        drawText(
            w,
            line,
            x + 18.f,
            y + 65.f + (i - start) * 34.f,
            17,
            sf::Color(220, 225, 235));
    }

    if (total == 0)
    {
        drawText(
            w,
            "Waiting for heap operations...",
            x + 18.f, y + 75.f, 17,
            sf::Color(160, 175, 195));
    }

    drawText(
        w,
        "Events recorded: " + std::to_string(total),
        x + 18.f, y + 335.f, 16,
        sf::Color(150, 170, 195));
}

void SandboxScreen::drawButton(
    sf::RenderWindow& w,
    const std::string& label,
    float x,
    float y,
    bool enabled)
{
    sf::RectangleShape button({ 195.f, 45.f });

    button.setPosition({ x, y });

    if (enabled)
        button.setFillColor(sf::Color(45, 85, 135));
    else
        button.setFillColor(sf::Color(48, 52, 62));

    button.setOutlineThickness(2.f);

    button.setOutlineColor(
        enabled
        ? sf::Color(130, 180, 225)
        : sf::Color(85, 90, 105));

    w.draw(button);

    sf::Text text(font);

    text.setString(label);
    text.setCharacterSize(18);

    text.setFillColor(
        enabled
        ? sf::Color::White
        : sf::Color(125, 130, 140));

    sf::FloatRect bounds = text.getLocalBounds();

    text.setOrigin({
        bounds.position.x + bounds.size.x / 2.f,
        bounds.position.y + bounds.size.y / 2.f
        });

    text.setPosition({ x + 97.f, y + 22.f });

    w.draw(text);
}

void SandboxScreen::drawOperationPanel(sf::RenderWindow& w)
{
    float x1 = 950.f;
    float x2 = 1160.f;

    float y = 680.f;
    float gap = 52.f;

    drawText(
        w,
        "SANDBOX OPERATIONS",
        x1, 650.f, 22,
        sf::Color(110, 205, 255));

    bool hasElements = heap.getCount() > 0;
    bool hasSelection =
        selectedFirst >= 0 &&
        selectedFirst < heap.getCount();

    drawButton(w, "BUILD HEAP (B)",
        x1, y, inputMode == 1);

    drawButton(w, "INSERT (I)",
        x2, y, inputMode == 2);

    drawButton(w, "EXTRACT MAX (E)",
        x1, y + gap, hasElements);

    drawButton(w, "SIFT UP (U)",
        x2, y + gap, hasSelection);

    drawButton(w, "SIFT DOWN (D)",
        x1, y + gap * 2, hasSelection);

    drawButton(w, "SWAP (S)",
        x2, y + gap * 2,
        selectedFirst >= 0 && selectedSecond >= 0);

    drawButton(w, "HEAP SORT",
        x1, y + gap * 3, hasElements);

    drawButton(w, "CLEAR",
        x2, y + gap * 3, true);

    drawButton(w, "RESET LAB",
        x1, y + gap * 4, true);

    drawButton(w, "BACK (ESC)",
        x2, y + gap * 4, true);
}

bool SandboxScreen::buttonHit(
    float mouseX,
    float mouseY,
    float x,
    float y) const
{
    return mouseX >= x &&
        mouseX <= x + 195.f &&
        mouseY >= y &&
        mouseY <= y + 45.f;
}

void SandboxScreen::handleButton(float mouseX, float mouseY)
{
    float x1 = 950.f;
    float x2 = 1160.f;

    float y = 680.f;
    float gap = 52.f;

    if (buttonHit(mouseX, mouseY, x1, y))
    {
        inputMode = 1;
        input.clear();
        feedback = "Type array values separated by spaces or commas, then press ENTER.";
        return;
    }

    if (buttonHit(mouseX, mouseY, x2, y))
    {
        inputMode = 2;
        input.clear();
        feedback = "Type one integer to insert, then press ENTER.";
        return;
    }

    if (buttonHit(mouseX, mouseY, x1, y + gap))
    {
        extractMaximum();
        return;
    }

    if (buttonHit(mouseX, mouseY, x2, y + gap))
    {
        siftUpSelected();
        return;
    }

    if (buttonHit(mouseX, mouseY, x1, y + gap * 2))
    {
        siftDownSelected();
        return;
    }

    if (buttonHit(mouseX, mouseY, x2, y + gap * 2))
    {
        swapSelected();
        return;
    }

    if (buttonHit(mouseX, mouseY, x1, y + gap * 3))
    {
        heapSort();
        return;
    }

    if (buttonHit(mouseX, mouseY, x2, y + gap * 3))
    {
        int empty[1] = {};

        heap.setRawArray(empty, 0);
        log.clear();

        selectedFirst = -1;
        selectedSecond = -1;
        extractedCount = 0;

        feedback = "Heap cleared.";
        return;
    }

    if (buttonHit(mouseX, mouseY, x1, y + gap * 4))
    {
        resetLab();
        return;
    }

    if (buttonHit(mouseX, mouseY, x2, y + gap * 4))
    {
        exitFlag = true;
        return;
    }
}

void SandboxScreen::handleTextInput(
    const sf::Event::TextEntered& textEvent)
{
    if (inputMode == 0)
        return;

    unsigned int character = textEvent.unicode;

    if (character == 8)
    {
        if (!input.empty())
            input.erase(input.length() - 1);

        return;
    }

    if (character == 13)
    {
        if (inputMode == 1)
            buildHeap();
        else if (inputMode == 2)
            insertValue();

        return;
    }

    if (character >= '0' && character <= '9')
    {
        if (input.length() < 150)
            input.push_back(static_cast<char>(character));

        return;
    }

    if (character == '-' || character == '+')
    {
        if (input.empty() ||
            input[input.length() - 1] == ' ' ||
            input[input.length() - 1] == ',')
        {
            if (input.length() < 150)
                input.push_back(static_cast<char>(character));
        }

        return;
    }

    if (character == ',' || character == ' ')
    {
        if (!input.empty() &&
            input[input.length() - 1] != ' ' &&
            input[input.length() - 1] != ',')
        {
            if (input.length() < 150)
                input.push_back(static_cast<char>(character));
        }
    }
}

void SandboxScreen::handleEvent(const sf::Event& e)
{
    if (const auto* key =
        e.getIf<sf::Event::KeyPressed>())
    {
        if (key->code == sf::Keyboard::Key::Escape)
        {
            exitFlag = true;
            return;
        }

        if (key->code == sf::Keyboard::Key::Enter)
        {
            if (inputMode == 1)
                buildHeap();
            else if (inputMode == 2)
                insertValue();

            return;
        }

        if (key->code == sf::Keyboard::Key::Backspace)
        {
            if (inputMode != 0 && !input.empty())
                input.erase(input.length() - 1);

            return;
        }

        if (key->code == sf::Keyboard::Key::B)
        {
            inputMode = 1;
            input.clear();
            feedback = "Enter array values, then press ENTER.";
        }
        else if (key->code == sf::Keyboard::Key::I)
        {
            inputMode = 2;
            input.clear();
            feedback = "Enter one integer, then press ENTER.";
        }
        else if(key->code == sf::Keyboard::Key::E){
        extractMaximum();
        }
        else if (key->code == sf::Keyboard::Key::U)
        {
            siftUpSelected();
        }
        else if (key->code == sf::Keyboard::Key::D)
        {
            siftDownSelected();
        }
        else if (key->code == sf::Keyboard::Key::S)
        {
            swapSelected();
        }
        else if (key->code == sf::Keyboard::Key::R)
        {
            resetLab();
        }
    }

    if (const auto* textEvent =
        e.getIf<sf::Event::TextEntered>())
    {
        handleTextInput(*textEvent);
    }
    if (const auto* mouse =
        e.getIf<sf::Event::MouseButtonPressed>())
    {
        if (mouse->button != sf::Mouse::Button::Left)
            return;

        float x = static_cast<float>(mouse->position.x);
        float y = static_cast<float>(mouse->position.y);

        if (x >= 950.f && x <= 1355.f &&
            y >= 680.f && y <= 950.f)
        {
            handleButton(x, y);
            return;
        }

        float startX = 950.f;
        float boxY = 335.f;
        float boxWidth = 56.f;
        float boxHeight = 65.f;
        float boxGap = 6.f;
        if (y >= boxY && y <= boxY + boxHeight)
        {
            for (int i = 0; i < heap.getCount(); i++)
            {
                float boxX =
                    startX + i * (boxWidth + boxGap);

                if (x >= boxX &&
                    x <= boxX + boxWidth)
                {
                    if (selectedFirst == -1)
                    {
                        selectedFirst = i;
                        selectedSecond = -1;

                        feedback =
                            "Selected heap index " +
                            std::to_string(i) + ".";
                    }
                    else if (selectedFirst == i)
                    {
                        selectedFirst = -1;
                        selectedSecond = -1;

                        feedback = "Selection cleared.";
                    }
                    else if (selectedSecond == -1)
                    {
                        selectedSecond = i;

                        feedback =
                            "Selected indices " +
                            std::to_string(selectedFirst) +
                            " and " + std::to_string(selectedSecond) + ".";
                    }
                    else
                    {
                        selectedFirst = i;
                        selectedSecond = -1;

                        feedback =
                            "Selection restarted at index " +
                            std::to_string(i) + ".";
                    }

                    return;
                }
            }
        }
    }
}

void SandboxScreen::update(float dt)
{
    (void)dt;
}

void SandboxScreen::draw(sf::RenderWindow& w)
{
    w.setView(w.getDefaultView());

    drawText(
        w,
        "SANDBOX - MAX HEAP LAB",
        55.f, 25.f, 40);

    drawText(
        w,
        "Build, modify, and experiment with a Max Heap.",
        60.f, 85.f, 25,
        sf::Color(110, 205, 255));

    drawText(
        w,
        feedback,
        60.f, 130.f, 19,
        sf::Color(225, 225, 190));
    drawText(
        w,
        "INPUT",
        60.f, 190.f, 20,
        sf::Color(110, 205, 255));

    sf::RectangleShape inputBox({ 790.f, 48.f });

    inputBox.setPosition({ 60.f, 220.f });
    inputBox.setFillColor(sf::Color(24, 30, 45));
    inputBox.setOutlineThickness(2.f);
    inputBox.setOutlineColor(
        inputMode != 0
        ? sf::Color(255, 190, 70)
        : sf::Color(95, 120, 155));

    w.draw(inputBox);

    std::string inputLabel = input;

    if (inputMode == 1)
        inputLabel = "BUILD: " + input;
    else if (inputMode == 2)
        inputLabel = "INSERT: " + input;
    else if (input.empty())
        inputLabel = "Press B to enter array values or I to insert one value";
    drawText(
        w,
        inputLabel,
        78.f, 230.f, 20,
        sf::Color(225, 230, 240));

    drawText(
        w,
        "MAX HEAP TREE",
        170.f, 185.f, 23,
        sf::Color(110, 205, 255));

    drawHeapTree(w);
    drawArray(w);
    drawEventLog(w);
    drawOperationPanel(w);

    drawText(
        w,
        "Click array cells to select elements. B: build | I: insert | E: extract",
        60.f, 960.f, 19,
        sf::Color(180, 195, 215));

    drawText(
        w,
        "U: sift up | D: sift down | S: swap | R: reset | ENTER: confirm input | ESC: exit",
        60.f, 995.f, 19,
        sf::Color(180, 195, 215));
}

bool SandboxScreen::exitRequested() const
{
    return exitFlag;
}