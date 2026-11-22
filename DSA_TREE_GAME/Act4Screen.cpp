#include "Act4Screen.h"
#include "Act4LevelGenerator.h"
#include <cmath>

Act4Screen::Act4Screen(
    sf::RenderWindow& gameWindow,
    int selectedMission,
    unsigned int seed)
{
    (void)gameWindow;

    font.openFromFile("arial.ttf");

    mission = selectedMission;
    levelSeed = seed;

    exitFlag = false;
    retryFlag = false;

    missionDone = false;
    missionFailed = false;

    historyCount = 0;
    historyIndex = 0;

    replaying = false;
    replayTimer = 0.f;
    replayDelay = 0.55f;

    animatingSwap = false;
    swapTimer = 0.f;
    swapDuration = 0.4f;

    compareA = -1;
    compareB = -1;

    resetMission();
}

void Act4Screen::resetMission()
{
    selectedFirst = -1;
    selectedSecond = -1;

    extractedCount = 0;
    operations = 0;

    timer = 0.f;
    timeLimit = 0.f;
    operationLimit = 0;

    exitFlag = false;
    retryFlag = false;

    missionDone = false;
    missionFailed = false;

    replaying = false;
    replayTimer = 0.f;

    animatingSwap = false;
    swapTimer = 0.f;

    compareA = -1;
    compareB = -1;

    totalCount = 7;
    optimalSteps = 7;

    int values[32] = {};

    Act4LevelGenerator::generate(
        mission,
        levelSeed,
        values,
        totalCount,
        optimalSteps
    );

    if (mission == 4)
    {
        timeLimit = 45.f;
        operationLimit = 12;
    }

    for (int i = 0; i < totalCount; i++)
        initialValues[i] = values[i];

    log.clear();
    alarm.start(optimalSteps);

    if (mission == 1 || mission == 2)
        heap.setRawArray(values, totalCount);
    else
        heap.buildMaxHeap(values, totalCount, &log);

    if (mission == 1)
        feedback = "Build a Max Heap. Select two elements and press S to swap.";

    else if (mission == 2)
        feedback = "Repair the heap. Select index 1 and press U to sift up.";

    else if (mission == 3)
        feedback = "Extract the maximum repeatedly using E.";

    else if (mission == 4)
        feedback = "Defeat the guards! Extract every maximum before time runs out.";

    else
        feedback = "Final Boss: extract every maximum to complete Heap Sort.";

    saveHistory();
}

void Act4Screen::drawText(
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

sf::Vector2f Act4Screen::getNodePosition(int index) const
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

void Act4Screen::drawLine(
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

void Act4Screen::drawHeapTree(sf::RenderWindow& w)
{
    int count = heap.getCount();

    // Find the most recent comparison event.
    compareA = -1;
    compareB = -1;

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

    // Draw parent-child relationships.
    for (int i = 1; i < count; i++)
    {
        int parent = (i - 1) / 2;

        sf::Color edgeColor(105, 125, 155);

        if (heap.getValue(parent) < heap.getValue(i))
            edgeColor = sf::Color(235, 65, 65);

        if ((parent == compareA && i == compareB) ||
            (parent == compareB && i == compareA))
            edgeColor = sf::Color(255, 205, 65);

        sf::Vector2f p1 = getNodePosition(parent);
        sf::Vector2f p2 = getNodePosition(i);

        drawLine(w, p1, p2, edgeColor);
    }

    // Draw nodes.
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

        if (animatingSwap &&
            (i == animIndexA || i == animIndexB))
            continue;

        drawText(w,
            std::to_string(heap.getValue(i)),
            position.x - 15.f,
            position.y - 16.f,
            24);
    }

    // Animate the two values travelling between their nodes.
    if (animatingSwap)
    {
        float progress = swapTimer / swapDuration;

        if (progress < 0.f)
            progress = 0.f;

        if (progress > 1.f)
            progress = 1.f;

        float ax =
            animStartA.x +
            (animStartB.x - animStartA.x) * progress;

        float ay =
            animStartA.y +
            (animStartB.y - animStartA.y) * progress;

        float bx =
            animStartB.x +
            (animStartA.x - animStartB.x) * progress;

        float by =
            animStartB.y +
            (animStartA.y - animStartB.y) * progress;

        drawText(w, std::to_string(animValueA),
            ax - 15.f, ay - 16.f, 24,
            sf::Color::Yellow);

        drawText(w, std::to_string(animValueB),
            bx - 15.f, by - 16.f, 24,
            sf::Color::Yellow);
    }

    drawText(w, "ROOT = MAXIMUM",
        125.f, 250.f, 20,
        sf::Color(100, 230, 150));

    drawText(w, "RED EDGE = INVALID HEAP RELATIONSHIP",
        125.f, 275.f, 17,
        sf::Color(255, 105, 105));
}

void Act4Screen::drawArray(sf::RenderWindow& w)
{
    float startX = 950.f;
    float boxY = 335.f;

    float boxWidth = 56.f;
    float boxHeight = 65.f;
    float gap = 6.f;

    drawText(w, "ACTIVE HEAP ARRAY",
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

        drawText(w,
            std::to_string(heap.getValue(i)),
            x + 12.f, boxY + 15.f, 22);

        drawText(w,
            std::to_string(i),
            x + 22.f, boxY + 70.f, 16,
            sf::Color(180, 190, 205));
    }

    float sortedY = 500.f;

    drawText(w, "EXTRACTED / SORTED REGION",
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

        // Maxima are extracted in descending order.
        // Reversing them displays the ascending sorted region.
        int value = extracted[extractedCount - 1 - i];

        drawText(w, std::to_string(value),
            x + 12.f, sortedY + 15.f, 22);
    }

    drawText(w,
        "Active elements: " + std::to_string(count),
        startX, 590.f, 18,
        sf::Color(180, 195, 215));

    drawText(w,
        "Sorted elements: " + std::to_string(extractedCount),
        startX, 620.f, 18,
        sf::Color(120, 230, 160));
}

std::string Act4Screen::eventName(int type) const
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

void Act4Screen::drawEventLog(sf::RenderWindow& w)
{
    float x = 1430.f;
    float y = 275.f;

    sf::RectangleShape panel({ 440.f, 375.f });

    panel.setPosition({ x, y });
    panel.setFillColor(sf::Color(24, 30, 45));
    panel.setOutlineThickness(2.f);
    panel.setOutlineColor(sf::Color(95, 120, 155));

    w.draw(panel);

    drawText(w, "EVENT LOG",
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

        drawText(w, line,
            x + 18.f,
            y + 65.f + (i - start) * 34.f,
            17,
            sf::Color(220, 225, 235));
    }

    if (total == 0)
    {
        drawText(w, "Waiting for heap operations...",
            x + 18.f, y + 75.f, 17,
            sf::Color(160, 175, 195));
    }

    drawText(w,
        "Events recorded: " + std::to_string(total),
        x + 18.f, y + 335.f, 16,
        sf::Color(150, 170, 195));
}

void Act4Screen::drawButton(
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

void Act4Screen::drawOperationPanel(sf::RenderWindow& w)
{
    float x1 = 950.f;
    float x2 = 1160.f;

    float y1 = 680.f;
    float rowGap = 52.f;

    drawText(w, "OPERATION PANEL",
        x1, 650.f, 22,
        sf::Color(110, 205, 255));

    bool active = !missionDone && !missionFailed && !replaying;

    drawButton(w, "SWAP (S)",
        x1, y1,
        active && mission == 1);

    drawButton(w, "SIFT UP (U)",
        x2, y1,
        active && mission == 2);

    drawButton(w, "SIFT DOWN (D)",
        x1, y1 + rowGap,
        active && mission == 2);

    drawButton(w, "BUILD HEAP",
        x2, y1 + rowGap,
        active && (mission == 1 || mission == 2));

    drawButton(w, "HEAPIFY",
        x1, y1 + rowGap * 2,
        active && (mission == 1 || mission == 2));

    drawButton(w, "EXTRACT MAX",
        x2, y1 + rowGap * 2,
        active && (mission == 3 || mission == 4 || mission == 5));

    drawButton(w, "HEAP SORT",
        x1, y1 + rowGap * 3,
        active && mission == 5);

    drawButton(w, "UNDO",
        x2, y1 + rowGap * 3,
        !replaying && historyIndex > 0 && !missionFailed);

    drawButton(w, "REPLAY",
        x1, y1 + rowGap * 4,
        !replaying && historyCount > 1);

    drawButton(w, "RESET LEVEL",
        x2, y1 + rowGap * 4,
        !replaying);
}

bool Act4Screen::buttonHit(
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

void Act4Screen::consumeAction()
{
    operations++;

    alarm.addSteps(1);

    if (alarm.isFailed())
    {
        missionFailed = true;
        feedback = "ALARM TRIGGERED! Security tolerance exceeded.";
    }
}

void Act4Screen::performSwap()
{
    if (mission != 1 ||
        selectedFirst < 0 ||
        selectedSecond < 0 ||
        selectedFirst == selectedSecond)
        return;

    int count = heap.getCount();

    if (selectedFirst >= count ||
        selectedSecond >= count)
        return;

    animValueA = heap.getValue(selectedFirst);
    animValueB = heap.getValue(selectedSecond);

    animIndexA = selectedFirst;
    animIndexB = selectedSecond;

    animStartA = getNodePosition(selectedFirst);
    animStartB = getNodePosition(selectedSecond);

    if (!heap.swapElements(
        selectedFirst, selectedSecond, &log))
        return;

    animatingSwap = true;
    swapTimer = 0.f;

    selectedFirst = -1;
    selectedSecond = -1;

    consumeAction();

    if (!missionFailed)
        checkCompletion();

    saveHistory();
}

void Act4Screen::performExtraction()
{
    if (missionDone ||
        missionFailed ||
        heap.getCount() <= 0)
        return;

    int maximum = heap.extractMax(&log);

    if (maximum == -99)
        return;

    if (extractedCount >= 32)
        return;

    extracted[extractedCount++] = maximum;

    consumeAction();

    if (!missionFailed)
        checkCompletion();

    saveHistory();
}

void Act4Screen::checkCompletion()
{
    if (missionDone || missionFailed)
        return;

    if (mission == 1 || mission == 2)
    {
        if (heap.isMaxHeap())
        {
            missionDone = true;
            feedback = "Max Heap property successfully restored!";
        }
        else
        {
            feedback = "Invalid relationship remains. Try another operation.";
        }
    }
    else
    {
        if (extractedCount == totalCount)
        {
            missionDone = true;

            if (mission == 5)
            {
                log.add(EventTypes::HEAP_SORT_COMPLETE);
                feedback = "HEAP SORT COMPLETE! Final Vault Unlocked!";
            }
            else
            {
                feedback = "All maximum elements extracted successfully!";
            }
        }
    }

    if (missionDone)
    {
        log.add(EventTypes::DONE);
        alarm.complete();
    }

    if (mission == 4 &&
        !missionDone &&
        operations >= operationLimit)
    {
        missionFailed = true;
        feedback = "Operation limit exceeded! The guards caught you.";
    }
}

void Act4Screen::saveHistory()
{
    if (historyIndex + 1 >= MAX_HISTORY)
        return;

    // Discard history after the current point if the player
    // performs a new action after Undo.
    historyCount = historyIndex + 1;

    Snapshot& snapshot = history[historyCount];

    snapshot.heapCount = heap.getCount();

    for (int i = 0; i < snapshot.heapCount; i++)
        snapshot.heap[i] = heap.getValue(i);

    snapshot.extractedCount = extractedCount;

    for (int i = 0; i < extractedCount; i++)
        snapshot.extracted[i] = extracted[i];

    snapshot.operations = operations;
    snapshot.alarmSteps = alarm.getAlarmSteps();

    snapshot.timer = timer;

    snapshot.missionDone = missionDone;
    snapshot.missionFailed = missionFailed;

    int eventCount = log.size();

    if (eventCount > MAX_EVENTS)
        eventCount = MAX_EVENTS;

    for (int i = 0; i < eventCount; i++)
        eventHistory[i] = log.get(i);

    snapshot.eventCount = eventCount;

    historyIndex = historyCount;
    historyCount++;
}

void Act4Screen::restoreSnapshot(
    const Snapshot& snapshot)
{
    heap.setRawArray(snapshot.heap, snapshot.heapCount);

    extractedCount = snapshot.extractedCount;

    for (int i = 0; i < extractedCount; i++)
        extracted[i] = snapshot.extracted[i];

    operations = snapshot.operations;
    timer = snapshot.timer;

    missionDone = snapshot.missionDone;
    missionFailed = snapshot.missionFailed;

    selectedFirst = -1;
    selectedSecond = -1;

    animatingSwap = false;

    alarm.start(optimalSteps);

    for (int i = 0; i < snapshot.alarmSteps; i++)
        alarm.addSteps(1);

    if (missionDone)
        alarm.complete();

    log.clear();

    for (int i = 0; i < snapshot.eventCount; i++)
    {
        const Events& event = eventHistory[i];

        log.add(
            event.type,
            event.id,
            event.extra,
            event.data);
    }
}

void Act4Screen::undo()
{
    if (replaying || historyIndex <= 0)
        return;

    historyIndex--;

    restoreSnapshot(history[historyIndex]);

    feedback = "Previous operation undone.";
}

void Act4Screen::startReplay()
{
    if (replaying || historyCount <= 1)
        return;

    replaying = true;
    replayTimer = 0.f;
    replayIndex = 1;

    restoreSnapshot(history[0]);

    replaying = true;
    feedback = "Replaying recorded operations...";
}

void Act4Screen::handleButton(float mouseX, float mouseY)
{
    if (replaying)
        return;

    float x1 = 950.f;
    float x2 = 1160.f;
    float y = 680.f;
    float gap = 52.f;

    if (buttonHit(mouseX, mouseY, x1, y))
    {
        if (mission == 1)
            performSwap();

        return;
    }

    if (buttonHit(mouseX, mouseY, x2, y))
    {
        if (mission == 2 &&
            selectedFirst >= 0 &&
            selectedFirst < heap.getCount())
        {
            heap.siftUp(selectedFirst, &log);

            selectedFirst = -1;

            consumeAction();

            if (!missionFailed)
                checkCompletion();

            saveHistory();
        }

        return;
    }

    if (buttonHit(mouseX, mouseY, x1, y + gap))
    {
        if (mission == 2 &&
            selectedFirst >= 0 &&
            selectedFirst < heap.getCount())
        {
            heap.siftDown(
                selectedFirst,
                heap.getCount(),
                &log);

            selectedFirst = -1;

            consumeAction();

            if (!missionFailed)
                checkCompletion();

            saveHistory();
        }

        return;
    }

    if (buttonHit(mouseX, mouseY, x2, y + gap))
    {
        if (mission == 1 || mission == 2)
        {
            heap.buildMaxHeap(
                initialValues,
                totalCount,
                &log);

            consumeAction();

            if (!missionFailed)
                checkCompletion();

            saveHistory();
        }

        return;
    }

    if (buttonHit(mouseX, mouseY, x1, y + gap * 2))
    {
        if ((mission == 1 || mission == 2) &&
            selectedFirst >= 0 &&
            selectedFirst < heap.getCount())
        {
            heap.siftDown(
                selectedFirst,
                heap.getCount(),
                &log);

            selectedFirst = -1;

            consumeAction();

            if (!missionFailed)
                checkCompletion();

            saveHistory();
        }

        return;
    }

    if (buttonHit(mouseX, mouseY, x2, y + gap * 2))
    {
        if (mission == 3 || mission == 4 || mission == 5)
            performExtraction();

        return;
    }

    if (buttonHit(mouseX, mouseY, x1, y + gap * 3))
    {
        if (mission == 5)
        {
            while (heap.getCount() > 0 &&
                !missionDone &&
                !missionFailed)
            {
                performExtraction();
            }
        }

        return;
    }

    if (buttonHit(mouseX, mouseY, x2, y + gap * 3))
    {
        undo();
        return;
    }

    if (buttonHit(mouseX, mouseY, x1, y + gap * 4))
    {
        startReplay();
        return;
    }

    if (buttonHit(mouseX, mouseY, x2, y + gap * 4))
    {
        resetMission();
        return;
    }
}

void Act4Screen::handleEvent(const sf::Event& e)
{
    if (const auto* key =
        e.getIf<sf::Event::KeyPressed>())
    {
        if (missionDone || missionFailed)
        {
            if (key->code == sf::Keyboard::Key::R)
                retryFlag = true;

            else if (key->code == sf::Keyboard::Key::Enter ||
                key->code == sf::Keyboard::Key::Escape)
                exitFlag = true;

            return;
        }

        if (key->code == sf::Keyboard::Key::Escape)
        {
            exitFlag = true;
            return;
        }

        if (key->code == sf::Keyboard::Key::R)
        {
            resetMission();
            return;
        }

        if (replaying)
            return;

        if (key->code == sf::Keyboard::Key::S)
            performSwap();

        else if (key->code == sf::Keyboard::Key::U &&
            mission == 2)
        {
            if (selectedFirst >= 0 &&
                selectedFirst < heap.getCount())
            {
                heap.siftUp(selectedFirst, &log);

                selectedFirst = -1;

                consumeAction();

                if (!missionFailed)
                    checkCompletion();

                saveHistory();
            }
        }

        else if (key->code == sf::Keyboard::Key::D &&
            mission == 2)
        {
            if (selectedFirst >= 0 &&
                selectedFirst < heap.getCount())
            {
                heap.siftDown(
                    selectedFirst,
                    heap.getCount(),
                    &log);

                selectedFirst = -1;

                consumeAction();

                if (!missionFailed)
                    checkCompletion();

                saveHistory();
            }
        }

        else if (key->code == sf::Keyboard::Key::E &&
            (mission == 3 || mission == 4 || mission == 5))
        {
            performExtraction();
        }

        else if (key->code == sf::Keyboard::Key::Z)
            undo();

        else if (key->code == sf::Keyboard::Key::P)
            startReplay();
    }

    if (const auto* mouse =
        e.getIf<sf::Event::MouseButtonPressed>())
    {
        if (mouse->button != sf::Mouse::Button::Left)
            return;

        if (missionDone || missionFailed)
            return;

        float x = static_cast<float>(mouse->position.x);
        float y = static_cast<float>(mouse->position.y);

        // Operation panel.
        if (x >= 950.f && x <= 1355.f &&
            y >= 680.f && y <= 950.f)
        {
            handleButton(x, y);
            return;
        }

        // Array selection.
        float startX = 950.f;
        float boxY = 335.f;
        float boxWidth = 56.f;
        float gap = 6.f;

        if (y >= boxY && y <= boxY + 65.f)
        {
            for (int i = 0; i < heap.getCount(); i++)
            {
                float boxX =
                    startX + i * (boxWidth + gap);

                if (x >= boxX &&
                    x <= boxX + boxWidth)
                {
                    if (selectedFirst == -1)
                    {
                        selectedFirst = i;
                        feedback = "First element selected.";
                    }
                    else if (selectedFirst == i)
                    {
                        selectedFirst = -1;
                    }
                    else
                    {
                        selectedSecond = i;
                        feedback = "Second element selected.";
                    }

                    return;
                }
            }
        }
    }
}

void Act4Screen::update(float dt)
{
    if (animatingSwap)
    {
        swapTimer += dt;

        if (swapTimer >= swapDuration)
        {
            animatingSwap = false;
            swapTimer = 0.f;
        }
    }

    if (replaying)
    {
        replayTimer += dt;

        if (replayTimer >= replayDelay)
        {
            replayTimer = 0.f;

            if (replayIndex < historyCount)
            {
                restoreSnapshot(history[replayIndex]);
                replayIndex++;
                replaying = true;
            }
            else
            {
                replaying = false;
                feedback = "Replay complete.";
            }
        }

        return;
    }

    if (missionDone || missionFailed || mission != 4)
        return;

    timer += dt;

    if (timer >= timeLimit)
    {
        missionFailed = true;
        feedback = "Time limit exceeded! The guards caught you.";
    }
}

void Act4Screen::drawHUD(sf::RenderWindow& w)
{
    drawText(w,
        "ACT 4 - THE HEAP FORTRESS",
        55.f, 25.f, 40);

    std::string missionName;

    switch (mission)
    {
    case 1: missionName = "4.1 - Activate the Vault"; break;
    case 2: missionName = "4.2 - Repair the Security Locks"; break;
    case 3: missionName = "4.3 - Extract the Greatest Treasure"; break;
    case 4: missionName = "4.4 - Race Against the Guards"; break;
    case 5: missionName = "4.5 - Final Vault Boss"; break;
    }

    drawText(w, missionName,
        60.f, 85.f, 28,
        sf::Color(110, 205, 255));

    drawText(w, feedback,
        60.f, 130.f, 19,
        missionFailed
        ? sf::Color(255, 100, 100)
        : sf::Color(225, 225, 190));

    drawText(w,
        "Steps: " + std::to_string(alarm.getAlarmSteps()) +
        " / " + std::to_string(alarm.getAlarmCapacity()),
        1450.f, 35.f, 22,
        sf::Color(255, 190, 90));

    drawText(w,
        "Remaining: " + std::to_string(alarm.getRemainingAlarm()),
        1450.f, 70.f, 18);

    drawText(w,
        "Score: " + std::to_string(alarm.getScore()),
        1450.f, 100.f, 18);

    float barX = 1450.f;
    float barY = 140.f;
    float barWidth = 350.f;
    float barHeight = 19.f;

    sf::RectangleShape background({ barWidth, barHeight });

    background.setPosition({ barX, barY });
    background.setFillColor(sf::Color(55, 60, 75));

    w.draw(background);

    float ratio = 0.f;

    if (alarm.getAlarmCapacity() > 0)
    {
        ratio =
            static_cast<float>(alarm.getAlarmSteps()) /
            static_cast<float>(alarm.getAlarmCapacity());
    }

    if (ratio < 0.f)
        ratio = 0.f;

    if (ratio > 1.f)
        ratio = 1.f;

    sf::RectangleShape bar({
        barWidth * ratio,
        barHeight
        });

    bar.setPosition({ barX, barY });

    if (ratio < 0.5f)
        bar.setFillColor(sf::Color(60, 190, 110));

    else if (ratio < 0.8f)
        bar.setFillColor(sf::Color(235, 165, 45));

    else
        bar.setFillColor(sf::Color(220, 65, 65));

    w.draw(bar);

    drawText(w,
        "Penalties: " +
        std::to_string(alarm.getPenaltyCount()),
        1450.f, 175.f, 18);

    if (mission == 4)
    {
        int remaining = static_cast<int>(timeLimit - timer);

        if (remaining < 0)
            remaining = 0;

        drawText(w,
            "TIME: " + std::to_string(remaining) +
            "s / 45s",
            1450.f, 210.f, 20,
            sf::Color(255, 105, 105));
    }

    drawText(w,
        "Mouse: select array positions | S: swap | U/D: sift",
        60.f, 960.f, 19,
        sf::Color(180, 195, 215));

    drawText(w,
        "E: extract | Z: undo | P: replay | R: reset | ESC: exit",
        60.f, 995.f, 19,
        sf::Color(180, 195, 215));
}

void Act4Screen::drawResultOverlay(sf::RenderWindow& w)
{
    sf::RectangleShape overlay({ 1920.f, 1080.f });

    overlay.setPosition({ 0.f, 0.f });
    overlay.setFillColor(sf::Color(5, 8, 15, 225));

    w.draw(overlay);

    if (missionDone)
    {
        drawText(w,
            "MISSION COMPLETE!",
            630.f, 210.f, 62,
            sf::Color(80, 255, 140));

        if (mission == 5)
        {
            drawText(w,
                "FINAL VAULT UNLOCKED",
                640.f, 290.f, 38,
                sf::Color(255, 215, 75));
        }

        drawText(w,
            "Final Score: " + std::to_string(alarm.getScore()),
            750.f, 385.f, 29);

        drawText(w,
            "Steps: " + std::to_string(alarm.getAlarmSteps()),
            750.f, 435.f, 25);

        drawText(w,
            "Penalties: " +
            std::to_string(alarm.getPenaltyCount()),
            750.f, 480.f, 25);

        drawText(w,
            "Stars: " + std::to_string(alarm.getStars()) + " / 3",
            750.f, 525.f, 30,
            sf::Color(255, 215, 75));

        drawText(w,
            "SORTED ARRAY",
            750.f, 590.f, 23,
            sf::Color(100, 230, 150));

        for (int i = 0; i < extractedCount; i++)
        {
            float x = 650.f + i * 90.f;

            sf::RectangleShape box({ 75.f, 55.f });

            box.setPosition({ x, 635.f });
            box.setFillColor(sf::Color(35, 125, 85));
            box.setOutlineThickness(2.f);
            box.setOutlineColor(sf::Color::White);

            w.draw(box);

            int value = extracted[extractedCount - 1 - i];

            drawText(w, std::to_string(value),
                x + 22.f, 648.f, 22);
        }
    }
    else
    {
        drawText(w,
            "MISSION FAILED",
            680.f, 300.f, 60,
            sf::Color(255, 75, 75));

        drawText(w, feedback,
            550.f, 405.f, 26,
            sf::Color(255, 180, 180));

        drawText(w,
            "Steps: " + std::to_string(alarm.getAlarmSteps()),
            760.f, 475.f, 26);

        drawText(w,
            "Score: " + std::to_string(alarm.getScore()),
            760.f, 520.f, 26);
    }

    drawText(w,
        "R - Retry Mission",
        755.f, 790.f, 28);

    drawText(w,
        "ENTER / ESC - Return to Campaign",
        640.f, 845.f, 27);
}

void Act4Screen::draw(sf::RenderWindow& w)
{
    w.setView(w.getDefaultView());

    drawHUD(w);

    drawText(w, "MAX HEAP TREE",
        170.f, 185.f,23,
        sf::Color(110, 205, 255));

    drawHeapTree(w);
    drawArray(w);
    drawEventLog(w);
    drawOperationPanel(w);

    if (missionDone || missionFailed)
        drawResultOverlay(w);
}

bool Act4Screen::exitRequested() const
{
    return exitFlag;
}

bool Act4Screen::retryRequested() const
{
    return retryFlag;
}

bool Act4Screen::isMissionDone() const
{
    return missionDone;
}

const AlarmSystem& Act4Screen::getAlarm() const
{
    return alarm;
}