#include"Act1.h"
#include"Act1LevelGenerator.h"
#include <fstream>
#include<cmath>

Act1::Act1(BinaryTree* t, unsigned int seed) {
levelSeed = seed;
    if (!font.openFromFile("arial.ttf"))
        cout << "SFML FAILED to load font\n";
    else
        cout << "SFML LOADED font successfully\n";
	tree = t;
    positions = nullptr;
    positionCount = 0;
    capacity = 0;

    firstClue = nullptr;
    secondClue = nullptr;

    comparisonDone = false;
    treesMatch = false;
    log = nullptr;
    eventDelay = 0.7f;
    currentEvent = 0;
    eventTimer = 0.f;
    playing = false;
    treeView = sf::View(sf::FloatRect(
        sf::Vector2f(0.f, 0.f),
        sf::Vector2f(1920.f, 1080.f)
    ));

    dragging = false;
    zoomLevel = 1.f;
    selectedNode = -1;
    nodeSelected = false;
    currentPath = nullptr;
    firstNode = -1;
    secondNode = -1;
    firstNodeSelected = false;
    secondNodeSelected = false;
    lcaMode = false;
    lcaNode = nullptr;
    lcaFound = false;
    symmetryChecked = false;
    symmetric = false;
    inputActive = false;
    inputBuffer = "";
    inputPrompt = "";
    inputStep = 0;
    inputID = -1;
    inputValue = 0;
    inputParentID = -1;
    inputIsLeft = true;
    currentMission = 0;
    missionActive = false;
    missionComplete = false;
    traversalChallenge = 0;
    expectedTraversal = nullptr;

    playerSequence = nullptr;
    playerCount = 0;
    playerCapacity = 0;
    missionFeedback = "";
    targetTree = nullptr;
    playerTree = nullptr;
    reconstructionRound = 0;
    sandboxTree = t;
    tokens = nullptr;
    tokenCount = 0;

    draggingToken = -1;
    dragOffset = sf::Vector2f(0.f, 0.f);
    heightTarget = nullptr;
    playerHeights = nullptr;
    heightNodeCount = 0;
    currentHeightIndex = 0;
    expectedDiameter = 0;
    playerDiameter = -1;
    heightNodeIDs = nullptr;
    heightDisplayTree = nullptr;
    selectedHeightNode = -1;
    heightInput = "";
    heightInputActive = false;
    diameterInputActive = false;
    missionTargetA = -1;
    missionTargetB = -1;
    expectedLCA = -1;

    lcaPhaseComplete = false;
    pathPhaseActive = false;

    expectedPath = nullptr;
    lcaDisplayTree = nullptr;
    playerPathCount = 0;
    playerPathCapacity = 8;
    playerPath = new int[playerPathCapacity];
    lcaSelectionPhase = false;
    mirrorTargetTree = nullptr;
    mirrorPlayerTree = nullptr;
    mirrorAutoMode = false;
}

void Act1::setEventLog(EventLog* l)
{
    log = l;
    currentEvent = 0;
    eventTimer = 0.f;
    playing = (log != nullptr && log->size() > 0);
}

void Act1::update(float deltaTime)
{
    if (!playing || log == nullptr)
        return;

    eventTimer += deltaTime;

    if (eventTimer >= eventDelay)
    {
        eventTimer -= eventDelay;
        currentEvent++;

        if (currentEvent >= log->size())
        {
            currentEvent = log->size() - 1;
            playing = false;
        }
    }
}

void Act1::addPosition(int id, Vector2f position)
{
    if (positionCount == capacity)
    {
        int newCapacity = (capacity == 0) ? 10 : capacity * 2;

        NodePosition* temp = new NodePosition[newCapacity];

        for (int i = 0; i < positionCount; i++)
            temp[i] = positions[i];

        delete[] positions;

        positions = temp;
        capacity = newCapacity;
    }

    positions[positionCount].id = id;
    positions[positionCount].position = position;

    positionCount++;
}

void Act1::calculatePositions(TreeNode* node, int depth, float& nextX)
{
    if (node == nullptr)
        return;

    calculatePositions(node->left, depth + 1, nextX);

    addPosition(
        node->id,
        sf::Vector2f(nextX, 150.f + depth * levelSpacing)
    );

    nextX += horizontalSpacing;

    calculatePositions(node->right, depth + 1, nextX);
}

Vector2f Act1::getPosition(int id)
{
    for (int i = 0; i < positionCount; i++)
    {
        if (positions[i].id == id)
            return positions[i].position;
    }

    return sf::Vector2f(0.f, 0.f);
}

void Act1::drawEdges(RenderWindow& window, TreeNode* node)
{
    if (node == nullptr)
        return;

    sf::Vector2f parentPos = getPosition(node->id);

    if (node->left != nullptr)
    {
        sf::Vector2f childPos = getPosition(node->left->id);

        sf::Vertex line[] =
        {
            sf::Vertex(parentPos),
            sf::Vertex(childPos)
        };

        window.draw(line, 2, sf::PrimitiveType::Lines);
    }

    if (node->right != nullptr)
    {
        sf::Vector2f childPos = getPosition(node->right->id);

        sf::Vertex line[] =
        {
            sf::Vertex(parentPos),
            sf::Vertex(childPos)
        };

        window.draw(line, 2, sf::PrimitiveType::Lines);
    }

    drawEdges(window, node->left);
    drawEdges(window, node->right);
}

void Act1::drawNodes(RenderWindow& window, TreeNode* node)
{
    if (node == nullptr)
        return;

    sf::Vector2f pos = getPosition(node->id);

    sf::CircleShape circle(nodeRadius);

    circle.setOrigin(
        sf::Vector2f(nodeRadius, nodeRadius)
    );

    circle.setPosition(pos);
    circle.setFillColor(getNodeColor(node->id));
    circle.setOutlineThickness(
        nodeSelected && selectedNode == node->id ? 5.f : 2.f
    );

    circle.setOutlineColor(
        nodeSelected && selectedNode == node->id
        ? sf::Color::Red
        : sf::Color::White
    );

    window.draw(circle);
    
    std::string displayValue =
        (missionActive &&
            currentMission == Missions::MISSION_1_2 &&
            node->val == Missions::EMPTY_SLOT)
        ? "?"
        : std::to_string(node->val);

    sf::Text valueText(font, displayValue, 22);
    sf::FloatRect bounds = valueText.getLocalBounds();

    valueText.setOrigin(
        bounds.position + bounds.size / 2.f
    );

    valueText.setPosition(pos);
    valueText.setFillColor(sf::Color::White);

    window.draw(valueText);
    drawNodes(window, node->left);
    drawNodes(window, node->right);
}

void Act1::draw(sf::RenderWindow& window)
{
    window.setView(treeView);

    positionCount = 0;

    float nextX = 80.f;
    calculatePositions(tree->getTree(), 0, nextX);

    drawEdges(window, tree->getTree());
    drawNodes(window, tree->getTree());

    window.setView(window.getDefaultView());

    drawMissionUI(window);
    if(!missionActive){
    drawEventInfo(window);
    drawCurrentTraversal(window);
    drawTraversalGuide(window);
    }
    drawSelectedNodeInfo(window);
    
    drawTreeStats(window);
    drawLCAInfo(window);
    drawSymmetryInfo(window);
    drawComparisonInfo(window);
    drawInput(window);
    drawTokens(window);
}

Color Act1::getNodeColor(int id)
{
    if (lcaMode)
    {
        if (firstNodeSelected && id == firstNode)
            return Color(255, 140, 0); 

        if (secondNodeSelected && id == secondNode)
            return Color(0, 220, 220); 
    }

   

    if (missionActive &&
        currentMission == Missions::MISSION_1_1)
    {
        for (int i = 0; i < playerCount; i++)
        {
            if (playerSequence[i] == id)
                return Color::Yellow;
        }

        return Color(40, 130, 200);
    }

    if (lcaFound && lcaNode != nullptr && id == lcaNode->id)
        return Color(255, 50, 50);

    if (isInCurrentPath(id))
        return Color(170, 80, 220);

    if (log == nullptr || log->size() == 0)
        return Color(40, 130, 200);

    if (currentEvent >= 0 && currentEvent < log->size())
    {
        const Events& current = log->get(currentEvent);

        if (current.type == EventTypes::VISIT && current.id == id)
            return Color::Yellow;
    }

    bool visited = false;

    for (int i = 0; i <= currentEvent; i++)
    {
        const Events& e = log->get(i);

        if (e.type == EventTypes::VISIT && e.id == id)
        {
            visited = true;
            break;
        }
    }

    if (visited)
        return Color::Green;

    return Color(40, 130, 200);
}

void Act1::pause()
{
    playing = false;
}

void Act1::resume()
{
    if (log != nullptr && log->size() > 0 &&
        currentEvent < log->size() - 1)
    {
        playing = true;
    }
}

void Act1::restart()
{
    if (log == nullptr || log->size() == 0)
        return;

    currentEvent = 0;
    eventTimer = 0.f;
    playing = true;
}

void Act1::setSpeed(float delay)
{
    if (delay > 0.f)
        eventDelay = delay;
}

void Act1::togglePlayback()
{
    if (playing)
        pause();
    else
        resume();
}

void Act1::nextEvent() {
    if (log == nullptr || log->size() == 0)
        return;
    playing = false;
    if(currentEvent < log->size()-1 && currentEvent >= 0) currentEvent++;
    eventTimer = 0.0f;
}

void Act1::previousEvent() {
    if (log == nullptr || log->size() == 0)
        return;
    pause();
    if(currentEvent > 0) currentEvent--;
    eventTimer = 0.f;
}

string Act1::getEventDescription()
{
    if (log == nullptr || log->size() == 0)
        return "No events available";

    if (currentEvent < 0 || currentEvent >= log->size())
        return "No event selected";

    const Events& e = log->get(currentEvent);

    switch (e.type)
    {
    case EventTypes::VISIT:
        return "VISIT: Node " + std::to_string(e.id);

    case EventTypes::COMPARE:
        return "COMPARE: Node " + std::to_string(e.id);

    case EventTypes::INSERT:
        return "INSERT: Node " + std::to_string(e.id);

    case EventTypes::UNLINK:
        return "UNLINK: Node " + std::to_string(e.id);

    case EventTypes::SWAP_VALUE:
        return "SWAP VALUE: Node " + std::to_string(e.id);

    case EventTypes::ROTATE:
        return "ROTATE: Node " + std::to_string(e.id);

    case EventTypes::UPDATE_HEIGHT:
        return "UPDATE HEIGHT: Node " + std::to_string(e.id);

    case EventTypes::UPDATE_SIZE:
        return "UPDATE SIZE: Node " + std::to_string(e.id);

    case EventTypes::BALANCE_ALERT:
        return "BALANCE ALERT: Node " + std::to_string(e.id);

    case EventTypes::HIGHLIGHT_SUBTREE:
        return "HIGHLIGHT SUBTREE: Node " + std::to_string(e.id);

    case EventTypes::HEAP_BUILD:
        return "HEAP BUILD";

    case EventTypes::HEAP_SWAP:
        return "HEAP SWAP";

    case EventTypes::HEAPIFY_START:
        return "HEAPIFY START";

    case EventTypes::HEAPIFY_DONE:
        return "HEAPIFY DONE";

    case EventTypes::EXTRACT_MAX:
        return "EXTRACT MAX";

    case EventTypes::MARK_SORTED:
        return "MARK SORTED";

    case EventTypes::HEAP_SIZE_CHANGE:
        return "HEAP SIZE CHANGE";

    case EventTypes::HEAP_SORT_COMPLETE:
        return "HEAP SORT COMPLETE";

    case EventTypes::DONE:
        return "OPERATION COMPLETE";

    default:
        return "UNKNOWN EVENT";
    }
}

void Act1::drawEventInfo(RenderWindow& window)
{
    sf::Text description(font, getEventDescription(), 20);
    description.setPosition(sf::Vector2f(20.f, 15.f));
    description.setFillColor(sf::Color::White);
    window.draw(description);

    std::string counter = "Event " +
        std::to_string(currentEvent + 1) + " / " +
        std::to_string(log != nullptr ? log->size() : 0);

    sf::Text eventCounter(font, counter, 16);
    eventCounter.setPosition(sf::Vector2f(20.f, 45.f));
    eventCounter.setFillColor(sf::Color(180, 180, 180));
    window.draw(eventCounter);

    std::string status = playing ? "PLAYING" : "PAUSED";

    sf::Text playbackStatus(font, status, 16);
    playbackStatus.setPosition(sf::Vector2f(1700.f, 15.f));
    playbackStatus.setFillColor(
        playing ? sf::Color::Green : sf::Color::Yellow
    );
    window.draw(playbackStatus);
}

void Act1::startDragging(sf::Vector2f mousePosition)
{
    dragging = true;
    lastMousePosition = mousePosition;
}

void Act1::stopDragging()
{
    dragging = false;
}

void Act1::dragView(sf::Vector2f mousePosition)
{
    if (!dragging)
        return;

    sf::Vector2f delta = lastMousePosition - mousePosition;

    treeView.move(delta);

    lastMousePosition = mousePosition;
}

void Act1::zoom(float delta)
{
    if (delta > 0.f && zoomLevel > 0.3f)
    {
        treeView.zoom(0.9f);
        zoomLevel *= 0.9f;
    }
    else if (delta < 0.f && zoomLevel < 3.f)
    {
        treeView.zoom(1.1f);
        zoomLevel *= 1.1f;
    }
}

void Act1::resetCamera()
{
    treeView.setCenter(sf::Vector2f(960.f, 540.f));
    treeView.setSize(sf::Vector2f(1920.f, 1080.f));

    zoomLevel = 1.f;
    dragging = false;
}

void Act1::selectNode(sf::Vector2f mousePosition, sf::RenderWindow& window)
{
    sf::Vector2i pixelPosition(
        static_cast<int>(mousePosition.x),
        static_cast<int>(mousePosition.y)
    );

    sf::Vector2f worldPosition =
        window.mapPixelToCoords(pixelPosition, treeView);

    int clickedNode = -1;

    for (int i = 0; i < positionCount; i++)
    {
        float dx = worldPosition.x - positions[i].position.x;
        float dy = worldPosition.y - positions[i].position.y;

        float distance = std::sqrt(dx * dx + dy * dy);

        if (distance <= nodeRadius)
        {
            clickedNode = positions[i].id;
            break;
        }
    }

    if (missionActive &&
        currentMission == Missions::MISSION_1_3)
    {
        TreeNode* clickedNode = nullptr;

        for (int i = 0; i < positionCount; i++)
        {
            float dx = worldPosition.x - positions[i].position.x;
            float dy = worldPosition.y - positions[i].position.y;

            if (std::sqrt(dx * dx + dy * dy) <= nodeRadius)
            {
                clickedNode = findNode(tree->getTree(), positions[i].id);
                break;
            }
        }

        if (currentHeightIndex >= heightNodeCount) return;

        if (clickedNode != nullptr)
        {
        missionActionCount++;

        

            int expectedID = heightNodeIDs[currentHeightIndex];

            if (clickedNode->id == expectedID)
            {
                selectedHeightNode = clickedNode->id;
                heightInput.clear();
                heightInputActive = true;

                missionFeedback = "Enter height for node " +
                    std::to_string(selectedHeightNode);

                feedbackClock.restart();
            }
            else
            {
                missionFeedback = "Wrong node! Follow postorder.";
                feedbackClock.restart();
            }
        }

        return;
    }


    if (!lcaMode)
    {
        selectedNode = clickedNode;
        nodeSelected = (clickedNode != -1);
        if(clickedNode != -1 && !(missionActive && currentMission == Missions::MISSION_1_4))
        missionActionCount++;

        if (missionActive &&
            currentMission == Missions::MISSION_1_1 &&
            clickedNode != -1)
        {
            recordNodeClick(clickedNode);
        }

        if (missionActive &&
            currentMission == Missions::MISSION_1_4 &&
            clickedNode != -1)
        {
            if (lcaSelectionPhase)
            {
                submitMeetingRoom(clickedNode);
                return;
            }

            if (pathPhaseActive)
            {
                recordPathClick(clickedNode);
            }
        }

        return;
    }

    if (clickedNode == -1)
        return;

        missionActionCount++;

    if (!firstNodeSelected || secondNodeSelected)
    {
        firstNode = clickedNode;
        firstNodeSelected = true;

        secondNode = -1;
        secondNodeSelected = false;
    }
    else
    {
        secondNode = clickedNode;
        secondNodeSelected = true;
    }
}

TreeNode* Act1::findNode(TreeNode* node, int id)
{
    if (node == nullptr)
        return nullptr;

    if (node->id == id)
        return node;

    TreeNode* found = findNode(node->left, id);

    if (found != nullptr)
        return found;

    return findNode(node->right, id);
}

string Act1::getSelectedNodeInfo()
{
    if (!nodeSelected)
        return "No node selected";

    TreeNode* node = findNode(tree->getTree(), selectedNode);

    if (node == nullptr)
        return "Node not found";

    return "ID: " + std::to_string(node->id) +
        "\nValue: " + std::to_string(node->val);
}

void Act1::drawSelectedNodeInfo(sf::RenderWindow& window)
{
    if (!nodeSelected)
        return;

    sf::Text title(font, "SELECTED NODE", 18);
    title.setPosition(sf::Vector2f(20.f, 850.f));
    title.setFillColor(sf::Color::Yellow);
    window.draw(title);

    sf::Text info(font, getSelectedNodeInfo(), 16);
    info.setPosition(sf::Vector2f(20.f, 880.f));
    info.setFillColor(sf::Color::White);
    window.draw(info);
}

void Act1::runTraversal(int type)
{
    if (tree == nullptr || log == nullptr)
        return;

    log->clear();
    currentTraversal = "";
    switch (type)
    {
    case 1:
    currentTraversal = "PREORDER (RECURSIVE)";
        tree->preOrder_R(log);
        break;

    case 2:
        currentTraversal = "PREORDER (ITERATIVE)";
        tree->preOrder_I(log);
        break;

    case 3:
        currentTraversal = "INORDER (RECURSIVE)";
        tree->inOrder_R(log);
        break;

    case 4:
        currentTraversal = "INORDER (ITERATIVE)";
        tree->inOrder_I(log);
        break;

    case 5:
        currentTraversal = "POSTORDER (RECURSIVE)";
        tree->postOrder_R(log);
        break;

    case 6:
        currentTraversal = "INORDER (ITERATIVE)";
        tree->postOrder_I(log);
        break;

    case 7:
        currentTraversal = "LEVELORDER (RECURSIVE)";
        tree->levelOrder_R(log);
        break;

    case 8:
        currentTraversal = "INORDER (ITERATIVE)";
        tree->levelOrder_I(log);
        break;

    default:
        return;
    }

    restart();
    setEventLog(log);
}

void Act1::drawTraversalGuide(sf::RenderWindow& window)
{
    sf::Text title(font, "TRAVERSALS", 18);
    title.setPosition(sf::Vector2f(1650.f, 850.f));
    title.setFillColor(sf::Color::Yellow);
    window.draw(title);

    std::string options =
        "1 - Preorder (Recursive)\n"
        "2 - Preorder (Iterative)\n"
        "3 - Inorder (Recursive)\n"
        "4 - Inorder (Iterative)\n"
        "5 - Postorder (Recursive)\n"
        "6 - Postorder (Iterative)\n"
        "7 - Level Order (Recursive)\n"
        "8 - Level Order (Iterative)";

    sf::Text guide(font, options, 14);
    guide.setPosition(sf::Vector2f(1650.f, 880.f));
    guide.setFillColor(sf::Color::White);
    window.draw(guide);
}

void Act1::drawCurrentTraversal(sf::RenderWindow& window)
{
    sf::Text mode(
        font,
        "CURRENT MODE: " + currentTraversal,
        18
    );

    mode.setPosition(sf::Vector2f(650.f, 15.f));
    mode.setFillColor(sf::Color::Cyan);

    window.draw(mode);
}

void Act1::mirrorTree()
{
    if (tree == nullptr)
        return;

    tree->mirror();

    nodeSelected = false;
    selectedNode = -1;

    positionCount = 0;
}

void Act1::drawTreeStats(sf::RenderWindow& window)
{
    if (tree == nullptr)
        return;

    int treeHeight = tree->height(tree->getTree());
    int treeDiameter = tree->diameter();

    sf::Text stats(font, "", 20);
    stats.setFillColor(sf::Color::White);
    stats.setPosition(sf::Vector2f(20.f, 200.f));

    stats.setString(
        "TREE HEIGHT: " + std::to_string(treeHeight) +
        "\nTREE DIAMETER: " + std::to_string(treeDiameter)
    );

    window.draw(stats);
}

void Act1::showPathToSelectedNode()
{
    if (currentPath != nullptr)
    {
        clearCurrentPath();
        return;
    }

    if (tree == nullptr || !nodeSelected)
        return;

    currentPath = tree->pathto(
        findNode(tree->getTree(), selectedNode)
    );
}

bool Act1::isInCurrentPath(int id)
{
    List* temp = currentPath;

    while (temp != nullptr)
    {
        if (temp->id == id)
            return true;

        temp = temp->next;
    }

    return false;
}

void Act1::clearCurrentPath()
{
    while (currentPath != nullptr)
    {
        List* temp = currentPath;
        currentPath = currentPath->next;
        delete temp;
    }
}

Act1::~Act1()
{
    clearCurrentPath();
    clearExpectedPath();

    while (expectedTraversal != nullptr)
    {
        List* temp = expectedTraversal;
        expectedTraversal = expectedTraversal->next;
        delete temp;
    }
    while (firstClue != nullptr)
    {
        List* temp = firstClue;
        firstClue = firstClue->next;
        delete temp;
    }
    while (secondClue != nullptr)
    {
        List* temp = secondClue;
        secondClue = secondClue->next;
        delete temp;
    }
    delete[] playerSequence;
    delete[] positions;
    delete targetTree;
    delete playerTree;
    delete[] heightNodeIDs;
    delete heightDisplayTree;
    delete lcaDisplayTree;
    delete[] playerPath;
    delete mirrorTargetTree;
    delete mirrorPlayerTree;
    delete[] tokens;
}

void Act1::toggleLCAMode()
{
    lcaMode = !lcaMode;
    lcaNode = nullptr;
    lcaFound = false;
    firstNodeSelected = false;
    secondNodeSelected = false;

    firstNode = -1;
    secondNode = -1;
}

void Act1::findLCA()
{
    if (!firstNodeSelected || !secondNodeSelected || tree == nullptr)
        return;

    lcaNode = tree->lca(firstNode, secondNode);
    lcaFound = (lcaNode != nullptr);
}

void Act1::drawLCAInfo(sf::RenderWindow& window)
{
    if (!lcaMode)
        return;

    std::string info = "LCA MODE";

    if (!firstNodeSelected)
        info += "\nSelect first node";
    else if (!secondNodeSelected)
        info += "\nSelect second node";
    else if (!lcaFound)
        info += "\nPress ENTER to find LCA";
    else
    {
        info += "\nLCA ID: " + std::to_string(lcaNode->id);
        info += "\nLCA Value: " + std::to_string(lcaNode->val);
    }

    sf::Text text(font, info, 20);
    text.setFillColor(Color::White);
    text.setPosition(sf::Vector2f(1650.f, 650.f));

    window.draw(text);
}

void Act1::checkSymmetry()
{
    if (tree == nullptr)
        return;

    symmetric = tree->isSymmetric();
    symmetryChecked = true;
}

void Act1::drawSymmetryInfo(sf::RenderWindow& window)
{
    if (!symmetryChecked)
        return;

    sf::Text text(font, "", 20);

    if (symmetric)
    {
        text.setString("TREE: SYMMETRIC");
        text.setFillColor(Color::Green);
    }
    else
    {
        text.setString("TREE: NOT SYMMETRIC");
        text.setFillColor(Color::Red);
    }

    text.setPosition(sf::Vector2f(1650.f, 750.f));
    window.draw(text);
}

void Act1::compareTree(TreeNode* otherRoot)
{
    if (tree == nullptr || otherRoot == nullptr)
        return;

    treesMatch = tree->isSameTree(otherRoot);
    comparisonDone = true;
}

void Act1::drawComparisonInfo(sf::RenderWindow& window)
{
    if (!comparisonDone)
        return;

    sf::Text text(font, "", 20);

    if (treesMatch)
    {
        text.setString("TREES ARE IDENTICAL");
        text.setFillColor(Color::Green);
    }
    else
    {
        text.setString("TREES ARE DIFFERENT");
        text.setFillColor(Color::Red);
    }

    text.setPosition(sf::Vector2f(1650.f, 800.f));
    window.draw(text);
}

void Act1::createNewTree()
{
clearCurrentPath();
tree->setTree(nullptr);
    tree->setTree(nullptr);

    currentPath = nullptr;
    selectedNode = -1;
    nodeSelected = false;

    firstNode = -1;
    secondNode = -1;
    firstNodeSelected = false;
    secondNodeSelected = false;

    lcaNode = nullptr;
    lcaFound = false;

    symmetryChecked = false;
    comparisonDone = false;

    positionCount = 0;

    if (log != nullptr)
        log->clear();

    currentEvent = 0;
    eventTimer = 0.f;
    playing = false;

    currentTraversal = "NONE";
}

bool Act1::createRoot(int id, int val)
{
    if (tree == nullptr || tree->getTree() != nullptr)
        return false;

    if (idExists(tree->getTree(), id))
        return false;

    tree->setTree(new TreeNode(id, val));
    positionCount = 0;

    return true;
}

bool Act1::addNode(int parentID, int id, int val, bool isLeft)
{
    if (tree == nullptr || tree->getTree() == nullptr)
        return false;

    if (idExists(tree->getTree(), id))
        return false;

    TreeNode* parent = findNode(tree->getTree(), parentID);

    if (parent == nullptr)
        return false;

    if (isLeft && parent->left != nullptr)
        return false;

    if (!isLeft && parent->right != nullptr)
        return false;

    TreeNode* newNode = new TreeNode(id, val);

    if (isLeft)
        parent->left = newNode;
    else
        parent->right = newNode;

    positionCount = 0;

    return true;
}

bool Act1::idExists(TreeNode* node, int id)
{
    if (node == nullptr)
        return false;

    if (node->id == id)
        return true;

    return idExists(node->left, id) ||
        idExists(node->right, id);
}

void Act1::handleTextInput(char32_t character)
{
    if (!inputActive)
        return;

    
    if (character == 8)
    {
        if (!inputBuffer.empty())
            inputBuffer.pop_back();

        return;
    }

    if (character >= '0' && character <= '9')
    {
        inputBuffer += static_cast<char>(character);
    }

    if (character == 13)
    {
        finishInput();
        return;
    }

    if (inputStep == 6)
    {
        if (character == '1' || character == '2')
        {
            inputIsLeft = (character == '1');

            if (addNode(inputParentID, inputID, inputValue, inputIsLeft))
                cancelInput();
            else
                cancelInput();
        }

        return;
    }
}

void Act1::startRootInput()
{
    if (tree == nullptr || tree->getTree() != nullptr)
        return;

    inputActive = true;
    inputStep = 1;
    inputBuffer = "";
    inputPrompt = "Enter Root ID:";
}

void Act1::startChildInput()
{
    if (tree == nullptr || tree->getTree() == nullptr)
        return;

    inputActive = true;
    inputStep = 3;
    inputBuffer = "";
    inputPrompt = "Enter Parent ID:";
}

void Act1::cancelInput()
{
    inputActive = false;
    inputBuffer = "";
    inputPrompt = "";
    inputStep = 0;
}

void Act1::finishInput()
{
    if (inputBuffer.empty())
        return;

    int value = std::stoi(inputBuffer);

    inputBuffer = "";

    switch (inputStep)
    {
    case 1:
        inputID = value;
        inputStep = 2;
        inputPrompt = "Enter Root Value:";
        break;

    case 2:
        inputValue = value;

        if (createRoot(inputID, inputValue))
            cancelInput();
        else
            cancelInput();

        break;

    case 3:
        inputParentID = value;
        inputStep = 4;
        inputPrompt = "Enter Child ID:";
        break;

    case 4:
        inputID = value;
        inputStep = 5;
        inputPrompt = "Enter Child Value:";
        break;

    case 5:
        inputValue = value;
        inputStep = 6;
        inputPrompt = "Choose 1 for Left or 2 for Right:";
        break;
    }
}

void Act1::handleInputKey(sf::Keyboard::Key key)
{
    if (!inputActive)
        return;

    if (key == sf::Keyboard::Key::Enter)
    {
        if (inputStep != 6)
            finishInput();

        return;
    }

    if (key == sf::Keyboard::Key::Backspace)
    {
        if (!inputBuffer.empty())
            inputBuffer.pop_back();

        return;
    }

    if (key == sf::Keyboard::Key::Escape)
    {
        cancelInput();
        return;
    }

   
}

void Act1::drawInput(sf::RenderWindow& window)
{
    if (!inputActive)
        return;

    sf::RectangleShape background(sf::Vector2f(500.f, 180.f));
    background.setFillColor(Color(30, 30, 40, 240));
    background.setOutlineColor(Color::White);
    background.setOutlineThickness(2.f);
    background.setPosition(sf::Vector2f(710.f, 400.f));

    window.draw(background);

    sf::Text prompt(font, inputPrompt, 24);
    prompt.setFillColor(Color::White);
    prompt.setPosition(sf::Vector2f(735.f, 430.f));

    window.draw(prompt);

    sf::Text input(font, inputBuffer + "_", 24);
    input.setFillColor(Color::Yellow);
    input.setPosition(sf::Vector2f(735.f, 485.f));

    window.draw(input);

    sf::Text instructions(font, "ENTER: Confirm    ESC: Cancel", 16);
    instructions.setFillColor(Color(180, 180, 180));
    instructions.setPosition(sf::Vector2f(735.f, 530.f));

    window.draw(instructions);
}

void Act1::setMission(int mission)
{
    if (mission < Missions::MISSION_1_1 ||
        mission > Missions::MISSION_1_5)
        return;

    currentMission = mission;
}

int Act1::getMission() const
{
    return currentMission;
}

void Act1::startMission()
{
    if (currentMission < Missions::MISSION_1_1 ||
        currentMission > Missions::MISSION_1_5)
        return;

    missionActive = true;
    missionComplete = false;

    if (currentMission == Missions::MISSION_1_1)
    {
        traversalChallenge = 0;
        prepareTraversalChallenge();
    }
    
    else if (currentMission == Missions::MISSION_1_2)
    {
        missionFeedback.clear();
        reconstructionRound = 0;
        prepareReconstructionChallenge();
    }
    else if (currentMission == Missions::MISSION_1_3)
    {
        prepareHeightChallenge();
    }
    else if (currentMission == Missions::MISSION_1_4)
    {
        prepareLCAChallenge();
    }
    else if (currentMission == Missions::MISSION_1_5) {
        prepareMirrorChallenge();
    }
}

bool Act1::isMissionActive() const
{
    return missionActive;
}

void Act1::completeMission()
{
    if (!missionActive)
        return;

    missionComplete = true;
    missionActive = false;
}

bool Act1::isMissionComplete() const
{
    return missionComplete;
}

void Act1::resetMission()
{
    missionActive = false;
    missionComplete = false;
}

void Act1::nextMission()
{
    if (!missionComplete)
        return;

    if (currentMission >= Missions::MISSION_1_5)
        return;

    currentMission++;

    missionActive = false;
    missionComplete = false;
}

void Act1::prepareTraversalChallenge()
{
    if (tree == nullptr || tree->getTree() == nullptr)
        return;

    while (expectedTraversal != nullptr)
    {
        List* temp = expectedTraversal;
        expectedTraversal = expectedTraversal->next;
        delete temp;
    }

    switch (traversalChallenge)
    {
    case 0:
        expectedTraversal = tree->preOrder_R();
        break;

    case 1:
        expectedTraversal = tree->inOrder_R();
        break;

    case 2:
        expectedTraversal = tree->postOrder_R();
        break;

    case 3:
        expectedTraversal = tree->levelOrder_R();
        break;

    default:
        return;
    }

    delete[] playerSequence;

    playerCapacity = countNodes(tree->getTree());
    if (playerCapacity < 1)
        playerCapacity = 1;

    playerSequence = new int[playerCapacity];
    playerCount = 0;
}

void Act1::recordNodeClick(int id)
{
    if (!missionActive ||
        currentMission != Missions::MISSION_1_1 ||
        expectedTraversal == nullptr)
        return;

    if (playerCount >= playerCapacity)
    {
        int newCapacity = playerCapacity * 2;
        int* temp = new int[newCapacity];

        for (int i = 0; i < playerCount; i++)
            temp[i] = playerSequence[i];

        delete[] playerSequence;

        playerSequence = temp;
        playerCapacity = newCapacity;
    }

    playerSequence[playerCount++] = id;
   
}

int Act1::getPlayerCount() const
{
    return playerCount;
}

bool Act1::checkTraversalAnswer()
{
    if (expectedTraversal == nullptr)
        return false;

    int expectedCount = 0;
    List* temp = expectedTraversal;

    while (temp != nullptr)
    {
        expectedCount++;
        temp = temp->next;
    }

    if (playerCount != expectedCount)
        return false;

    temp = expectedTraversal;

    for (int i = 0; i < playerCount; i++)
    {
        if (playerSequence[i] != temp->id)
            return false;

        temp = temp->next;
    }

    return true;
}

bool Act1::submitTraversalAnswer()
{
    if (!checkTraversalAnswer())
    {
        missionFeedback = "Incorrect sequence! Try again.";
        feedbackClock.restart();

        playerCount = 0;
        return false;
    }

    if (traversalChallenge == 3)
    {
        missionFeedback = "Mission Complete!";
        feedbackClock.restart();

        completeMission();
        return true;
    }

    missionFeedback = "Correct! Moving to the next traversal.";
    feedbackClock.restart();

    traversalChallenge++;
    prepareTraversalChallenge();
    return true;
}

void Act1::drawMissionUI(sf::RenderWindow& window)
{
   
    if (!missionActive && !missionComplete)
        return;
    sf::Text title(font, "THE BLUEPRINT WING", 28);
    title.setFillColor(sf::Color::Yellow);
    title.setPosition(sf::Vector2f(20.f, 20.f));
    window.draw(title);

    std::string objectiveText;

    if (currentMission == Missions::MISSION_1_1)
        objectiveText = "Mission 1.1: Predict the guards' patrol route";
    else if (currentMission == Missions::MISSION_1_2)
        objectiveText = "Mission 1.2: Decode the hidden blueprint";

    sf::Text objective(font, objectiveText, 20);
    objective.setFillColor(sf::Color::White);
    objective.setPosition(sf::Vector2f(20.f, 60.f));
    window.draw(objective);


    if(currentMission == Missions::MISSION_1_1){
    const char* traversalNames[] = {
    "PREORDER",
    "INORDER",
    "POSTORDER",
    "LEVEL ORDER"
    };

    sf::Text challenge(font, traversalNames[traversalChallenge], 24);
    challenge.setFillColor(sf::Color::Cyan);
    challenge.setPosition(sf::Vector2f(20.f, 100.f));
    window.draw(challenge);

    int expectedCount = 0;
    List* temp = expectedTraversal;

    while (temp != nullptr) {
        expectedCount++;
        temp = temp->next;
    }

    if (expectedTraversal != nullptr) {
        sf::Text progress(
            font,
            "Nodes selected: " + std::to_string(playerCount) +
            " / " + std::to_string(expectedCount),
            20
        );

        progress.setFillColor(sf::Color::White);
        progress.setPosition(sf::Vector2f(20.f, 140.f));
        window.draw(progress);
    }

    sf::Text instructions(
        font,
        "Click rooms in the correct order. Press ENTER to submit.",
        18
    );

    instructions.setFillColor(sf::Color(180, 180, 180));
    instructions.setPosition(sf::Vector2f(20.f, 180.f));
    window.draw(instructions);

    if (!missionFeedback.empty() &&
        feedbackClock.getElapsedTime().asSeconds() < 2.5f)
    {
        sf::Text feedback(font, missionFeedback, 20);
        feedback.setFillColor(sf::Color::Green);
        feedback.setPosition(sf::Vector2f(20.f, 250.f));
        window.draw(feedback);
    }
    }

    if (currentMission == Missions::MISSION_1_2)
    {
        // First traversal clue
        sf::Text firstLabel(
            font,
            reconstructionRound == 0 ? "PREORDER:" : "POSTORDER:",
            20
        );

        firstLabel.setFillColor(sf::Color::Yellow);
        firstLabel.setPosition(sf::Vector2f(20.f, 375.f));
        window.draw(firstLabel);

        // Second traversal clue
        sf::Text secondLabel(font, "INORDER:", 20);

        secondLabel.setFillColor(sf::Color::Cyan);
        secondLabel.setPosition(sf::Vector2f(20.f, 400.f));
        window.draw(secondLabel);

        // Draw first clue values
        int x = 150;

        for (List* curr = firstClue; curr != nullptr; curr = curr->next)
        {
            sf::Text value(font, std::to_string(curr->val), 20);
            value.setFillColor(sf::Color::White);
            value.setPosition(sf::Vector2f(float(x), 375.f));
            window.draw(value);

            x += 50;
        }

        // Draw second clue values
        x = 150;

        for (List* curr = secondClue; curr != nullptr; curr = curr->next)
        {
            sf::Text value(font, std::to_string(curr->val), 20);
            value.setFillColor(sf::Color::White);
            value.setPosition(sf::Vector2f(float(x), 400.f));
            window.draw(value);

            x += 50;
        }
    }

    if (currentMission == Missions::MISSION_1_3)
    {
        sf::Text round(font, "MISSION 1.3: ALARM SPREADS UPWARD", 22);
        round.setFillColor(sf::Color::Cyan);
        round.setPosition(sf::Vector2f(20.f, 100.f));
        window.draw(round);

        sf::Text progress(
            font,
            "Heights calculated: " +
            std::to_string(currentHeightIndex) +
            " / " + std::to_string(heightNodeCount),
            20
        );

        progress.setFillColor(sf::Color::White);
        progress.setPosition(sf::Vector2f(20.f, 145.f));
        window.draw(progress);

        std::string instruction;

        if (diameterInputActive)
        {
            instruction = "All heights correct! Enter the tree diameter:";
        }
        else if (heightInputActive)
        {
            instruction = "Enter height for node " +
                std::to_string(selectedHeightNode) + ":";
        }
        else if (currentHeightIndex < heightNodeCount)
        {
            instruction = "Select node " +
                std::to_string(heightNodeIDs[currentHeightIndex]);
        }

        sf::Text prompt(font, instruction, 20);
        prompt.setFillColor(sf::Color::Yellow);
        prompt.setPosition(sf::Vector2f(20.f, 250.f));
        window.draw(prompt);

        sf::Text input(font, heightInput, 24);
        input.setFillColor(sf::Color::Green);
        input.setPosition(sf::Vector2f(20.f, 275.f));
        window.draw(input);
    }

    if (currentMission == Missions::MISSION_1_4)
    {

        int targetAValue = getNodeValueByID(
            tree->getTree(), missionTargetA
        );

        int targetBValue = getNodeValueByID(
            tree->getTree(), missionTargetB
        );

        int lcaValue = getNodeValueByID(
            tree->getTree(), expectedLCA
        );



        sf::Text title(
            font,
            "MISSION 1.4: ESCAPE ROUTES",
            22
        );

        title.setFillColor(sf::Color::Cyan);
        title.setPosition(sf::Vector2f(20.f, 100.f));
        window.draw(title);

        sf::Text targets(
            font,
            "Find the LCA of rooms " +
            std::to_string(targetAValue) +
            " and " +
            std::to_string(targetBValue),
            20
        );

        targets.setFillColor(sf::Color::White);
        targets.setPosition(sf::Vector2f(20.f, 145.f));
        window.draw(targets);

        std::string instruction;

        if (!lcaPhaseComplete)
        {
            instruction = "Select both rooms, then press ENTER.";
        }
        else
        {
            instruction = "Trace the path from room " +
                std::to_string(targetAValue) +
                " to room " +
                std::to_string(targetBValue) +
                ". Press ENTER to submit.";
        }

        sf::Text prompt(font, instruction, 18);
        prompt.setFillColor(sf::Color::Yellow);
        prompt.setPosition(sf::Vector2f(20.f, 275.f));
        window.draw(prompt);

        if (pathPhaseActive)
        {
            sf::Text progress(
                font,
                "Rooms selected: " +
                std::to_string(playerPathCount),
                18
            );

            progress.setFillColor(sf::Color::Green);
            progress.setPosition(sf::Vector2f(20.f, 300.f));
            window.draw(progress);
        }

        if (lcaPhaseComplete)
        {
            sf::Text lcaResult(
                font,
                "Escape routes meet at room: " +
                std::to_string(lcaValue),
                18
            );

            lcaResult.setFillColor(sf::Color::Green);
            lcaResult.setPosition(sf::Vector2f(20.f, 330.f));
            window.draw(lcaResult);
        }

    }

    else if (currentMission == Missions::MISSION_1_5)
    {
        sf::Text title(font);
        title.setString("MISSION 1.5: THE MIRROR VAULT");
        title.setCharacterSize(24);
        title.setFillColor(sf::Color::White);
        title.setPosition(sf::Vector2f(30.f, 70.f));
        window.draw(title);

        sf::Text instruction(font);
        instruction.setString(
            "Mirror the tree by selecting a node and pressing M.\n"
            "Press A to automatically mirror the entire tree.\n"
            "Press ENTER to submit your answer."
        );
        instruction.setCharacterSize(18);
        instruction.setFillColor(sf::Color::White);
        instruction.setPosition(sf::Vector2f(30.f, 120.f));
        window.draw(instruction);
    }


    if (!missionFeedback.empty() &&
        feedbackClock.getElapsedTime().asSeconds() < 2.5f)
    {
        sf::Text feedback(font, missionFeedback, 20);

        feedback.setFillColor(sf::Color::Green);
        feedback.setPosition(sf::Vector2f(20.f, 425.f));

        window.draw(feedback);
    }
}

TreeNode* Act1::cloneBlueprint(TreeNode* node)
{
    if (node == nullptr)
        return nullptr;

    TreeNode* copy = new TreeNode(node->id, Missions::EMPTY_SLOT);

    copy->left = cloneBlueprint(node->left);
    copy->right = cloneBlueprint(node->right);

    return copy;
}

void Act1::collectTokenValues(TreeNode* node, int& index)
{
    if (node == nullptr)
        return;

    tokens[index].value = node->val;
    tokens[index].placed = false;
    tokens[index].nodeID = -1;

    index++;

    collectTokenValues(node->left, index);
    collectTokenValues(node->right, index);
}

int Act1::countNodes(TreeNode* node)
{
    if (node == nullptr)
        return 0;

    return 1 + countNodes(node->left) + countNodes(node->right);
}

void Act1::createTokens()
{
    delete[] tokens;

    tokenCount = countNodes(targetTree->getTree());
    tokens = new Token[tokenCount];

    draggingToken = -1;

    int index = 0;
    collectTokenValues(targetTree->getTree(), index);

    for (int i = 0; i < tokenCount / 2; i++)
    {
        int temp = tokens[i].value;
        tokens[i].value = tokens[tokenCount - 1 - i].value;
        tokens[tokenCount - 1 - i].value = temp;
    }

    int tokenStart = tokenCount > 10 ? 320 : Missions::TOKEN_START_X;
    int tokenSpacing = tokenCount > 10 ? 85 : Missions::TOKEN_SPACING;

    for (int i = 0; i < tokenCount; i++)
    {
        tokens[i].position = sf::Vector2f(
            static_cast<float>(tokenStart + i * tokenSpacing),
            static_cast<float>(Missions::TOKEN_START_Y)
        );
    }
}

void Act1::drawTokens(sf::RenderWindow& window)
{
    for (int i = 0; i < tokenCount; i++)
    {
        if (tokens[i].placed && draggingToken != i)
            continue;

        sf::CircleShape token(30.f);
        token.setOrigin(sf::Vector2f(30.f, 30.f));
        token.setPosition(tokens[i].position);
        token.setFillColor(sf::Color(180, 130, 50));
        token.setOutlineThickness(3.f);
        token.setOutlineColor(sf::Color::White);

        window.draw(token);

        sf::Text valueText(font, std::to_string(tokens[i].value), 22);
        valueText.setFillColor(sf::Color::White);

        sf::FloatRect bounds = valueText.getLocalBounds();
        valueText.setOrigin(sf::Vector2f(
            bounds.position.x + bounds.size.x / 2.f,
            bounds.position.y + bounds.size.y / 2.f
        ));

        valueText.setPosition(tokens[i].position);
        window.draw(valueText);
    }
}

void Act1::createSampleTarget()
{
    if (targetTree == nullptr)
        targetTree = new BinaryTree();

    unsigned int seed = SeededRandom::deriveSeed(
        levelSeed,
        2,
        static_cast<unsigned int>(reconstructionRound + 1)
    );

    targetTree->setTree(Act1LevelGenerator::generate(seed));
}

void Act1::prepareReconstructionChallenge()
{
draggingToken = -1;
    createSampleTarget();
    createPlayerTree();
    createTokens();
    prepareReconstructionClues();
    tree = playerTree;
}

void Act1::createPlayerTree()
{
    if (playerTree == nullptr)
        playerTree = new BinaryTree();

    playerTree->setTree(cloneBlueprint(targetTree->getTree()));

    List* test = playerTree->preOrder_R();

    while (test != nullptr)
    {
        std::cout << test->val << " ";

        List* temp = test;
        test = test->next;
        delete temp;
    }

    std::cout << '\n';
}

void Act1::startTokenDrag(sf::Vector2f mousePosition)
{
    if (!missionActive ||
        currentMission != Missions::MISSION_1_2)
        return;

    for (int i = 0; i < tokenCount; i++)
    {
        if (tokens[i].placed)
            continue;

        float dx = mousePosition.x - tokens[i].position.x;
        float dy = mousePosition.y - tokens[i].position.y;

        if (std::sqrt(dx * dx + dy * dy) <= 30.f)
        {
            draggingToken = i;
            dragOffset = tokens[i].position - mousePosition;
            return;
        }
    }
}

void Act1::updateTokenDrag(sf::Vector2f mousePosition)
{
    if (draggingToken == -1)
        return;

    tokens[draggingToken].position = mousePosition + dragOffset;
}

void Act1::dropToken(sf::Vector2f mousePosition, sf::RenderWindow& window)
{
    if (draggingToken == -1)
        return;

    sf::Vector2i pixelPosition(
        static_cast<int>(mousePosition.x),
        static_cast<int>(mousePosition.y)
    );

    sf::Vector2f worldPosition =
        window.mapPixelToCoords(pixelPosition, treeView);

    for (int i = 0; i < positionCount; i++)
    {
        float dx = worldPosition.x - positions[i].position.x;
        float dy = worldPosition.y - positions[i].position.y;

        if (std::sqrt(dx * dx + dy * dy) > nodeRadius)
            continue;

        TreeNode* node = findNode(playerTree->getTree(), positions[i].id);

        if (node != nullptr && node->val == Missions::EMPTY_SLOT)
        {
            node->val = tokens[draggingToken].value;

            tokens[draggingToken].placed = true;
            tokens[draggingToken].nodeID = node->id;
            missionActionCount++;
        }

        break;
    }

    draggingToken = -1;
}

bool Act1::allTokensPlaced()
{
    for (int i = 0; i < tokenCount; i++)
    {
        if (!tokens[i].placed)
            return false;
    }

    return true;
}

bool Act1::compareReconstruction(TreeNode* target, TreeNode* player)
{
    if (target == nullptr && player == nullptr)
        return true;

    if (target == nullptr || player == nullptr)
        return false;

    if (target->id != player->id || target->val != player->val)
        return false;

    return compareReconstruction(target->left, player->left) &&
        compareReconstruction(target->right, player->right);
}

bool Act1::submitReconstruction()
{
    if (!allTokensPlaced())
    {
        missionFeedback = "Place all the tokens first!";
        feedbackClock.restart();
        return false;
    }

    if (!compareReconstruction(targetTree->getTree(), playerTree->getTree()))
    {
        missionFeedback = "Incorrect reconstruction! Try again.";
        feedbackClock.restart();

        
       resetPlayerTree(playerTree->getTree());

        
        for (int i = 0; i < tokenCount; i++)
        {
            tokens[i].placed = false;
            tokens[i].nodeID = -1;

            tokens[i].position = sf::Vector2f(
                float(Missions::TOKEN_START_X + i * Missions::TOKEN_SPACING),
                float(Missions::TOKEN_START_Y)
            );
        }

        return false;
    }

    missionFeedback = "Correct reconstruction!";
    feedbackClock.restart();

    if (reconstructionRound == 0)
    {
        reconstructionRound = 1;
        prepareReconstructionChallenge();
    }
    else
    {
        missionFeedback = "Mission Complete!";
        completeMission();
    }

    return true;
}

void Act1::prepareReconstructionClues()
{
    while (firstClue != nullptr)
    {
        List* temp = firstClue;
        firstClue = firstClue->next;
        delete temp;
    }

    while (secondClue != nullptr)
    {
        List* temp = secondClue;
        secondClue = secondClue->next;
        delete temp;
    }

    if (reconstructionRound == 0)
    {
        firstClue = targetTree->preOrder_R();
    }
    else
    {
        firstClue = targetTree->postOrder_R();
    }

    secondClue = targetTree->inOrder_R();
}

void Act1::resetPlayerTree(TreeNode* node)
{
    if (node == nullptr)
        return;

    node->val = Missions::EMPTY_SLOT;

    resetPlayerTree(node->left);
    resetPlayerTree(node->right);
}

void Act1::collectPostorderNodes(TreeNode* node, int* ids, int& index)
{
    if (node == nullptr)
        return;

    collectPostorderNodes(node->left, ids, index);
    collectPostorderNodes(node->right, ids, index);

    ids[index++] = node->id;
}

void Act1::prepareHeightChallenge()
{
    delete[] heightNodeIDs;
    delete[] playerHeights;

    heightNodeCount = 0;
    currentHeightIndex = 0;
    playerDiameter = -1;

    if (heightTarget != nullptr)
    {
        // Recursively free the old generated tree.
        Act1LevelGenerator::destroyTree(heightTarget);
        heightTarget = nullptr;
    }

    unsigned int seed = SeededRandom::deriveSeed(
        levelSeed, 3, 1
    );

    heightTarget = Act1LevelGenerator::generate(seed);

    heightNodeCount = countNodes(heightTarget);

    heightNodeIDs = new int[heightNodeCount];
    playerHeights = new int[heightNodeCount];

    for (int i = 0; i < heightNodeCount; i++)
        playerHeights[i] = -1;

    int index = 0;
    collectPostorderNodes(heightTarget, heightNodeIDs, index);

    BinaryTree temp;
    temp.setTree(cloneBlueprint(heightTarget));

    expectedDiameter = temp.diameter();

    if (heightDisplayTree == nullptr)
        heightDisplayTree = new BinaryTree();

    heightDisplayTree->setTree(cloneTree(heightTarget));
    tree = heightDisplayTree;
}

void Act1::submitNodeHeight(int height)
{
    if (!missionActive ||
        currentMission != Missions::MISSION_1_3)
        return;

    if (currentHeightIndex >= heightNodeCount)
        return;

    missionActionCount++;

    TreeNode* node = findNode(
        heightTarget,
        heightNodeIDs[currentHeightIndex]
    );

    

    int expected = 1 + heightDisplayTree->max(
        heightDisplayTree->height(node->left),
        heightDisplayTree->height(node->right)
    );

    if (height != expected)
    {
        missionFeedback = "Incorrect height! Try again.";
        feedbackClock.restart();
        return;
    }

    playerHeights[currentHeightIndex] = height;
    currentHeightIndex++;

    missionFeedback = "Correct height!";
    feedbackClock.restart();

    if (currentHeightIndex == heightNodeCount)
    {
        diameterInputActive = true;
        heightInputActive = true;

        missionFeedback = "All heights correct! Enter the tree diameter.";
        feedbackClock.restart();
    }
}

TreeNode* Act1::cloneTree(TreeNode* node)
{
    if (node == nullptr)
        return nullptr;

    TreeNode* copy = new TreeNode(node->id, node->val);

    copy->left = cloneTree(node->left);
    copy->right = cloneTree(node->right);

    return copy;
}

void Act1::handleHeightTextInput(char character)
{
    if (!missionActive ||
        currentMission != Missions::MISSION_1_3 ||
        !heightInputActive)
        return;

    if (character >= '0' && character <= '9')
    {
        heightInput += character;
    }
    else if (character == '\b' && !heightInput.empty())
    {
        heightInput.pop_back();
    }
}

void Act1::submitHeightInput()
{
    if (!heightInputActive || heightInput.empty())
        return;

    int value = std::stoi(heightInput);

    if (diameterInputActive)
    {
        playerDiameter = value;
        missionActionCount++;

        if (playerDiameter == expectedDiameter)
        {
            missionFeedback = "Correct diameter! Mission Complete!";
            feedbackClock.restart();

            completeMission();
        }
        else
        {
            missionFeedback = "Incorrect diameter! Try again.";
            feedbackClock.restart();

            playerDiameter = -1;
        }
    }
    else
    {
        submitNodeHeight(value);
    }

    heightInput.clear();

    if (!diameterInputActive || missionComplete)
        heightInputActive = false;

    if (missionComplete)
        diameterInputActive = false;
}

bool Act1::checkHeightAnswers()
{
    for (int i = 0; i < heightNodeCount; i++)
    {
        TreeNode* node = findNode(heightTarget, heightNodeIDs[i]);

        if (node == nullptr)
            return false;

        int expected = 1 + heightDisplayTree->max(
            heightDisplayTree->height(node->left),
            heightDisplayTree->height(node->right)
        );

        if (playerHeights[i] != expected)
            return false;
    }

    return true;
}

void Act1::prepareLCAChallenge()
{
    unsigned int seed = SeededRandom::deriveSeed(
        levelSeed, 4, 1
    );

    TreeNode* root = Act1LevelGenerator::generate(seed);

    if (lcaDisplayTree == nullptr)
        lcaDisplayTree = new BinaryTree();

    lcaDisplayTree->setTree(root);
    tree = lcaDisplayTree;

    int count = countNodes(root);

    SeededRandom random(seed ^ 0xB5297A4D);

    missionTargetA = random.range(2, count);
    missionTargetB = random.range(2, count);

    while (missionTargetB == missionTargetA)
        missionTargetB = random.range(2, count);

    TreeNode* common = Act1LevelGenerator::findLCA(
        root, missionTargetA, missionTargetB
    );

    expectedLCA = common != nullptr ? common->id : root->id;

    lcaSelectionPhase = false;
    lcaPhaseComplete = false;
    pathPhaseActive = false;

    toggleLCAMode();

    lcaNode = nullptr;
    lcaFound = false;

    clearExpectedPath();
    buildExpectedPath();
}

void Act1::clearExpectedPath()
{
    while (expectedPath != nullptr)
    {
        List* temp = expectedPath;
        expectedPath = expectedPath->next;
        delete temp;
    }
}

bool Act1::submitLCAAnswer()
{
    if (!missionActive ||
        currentMission != Missions::MISSION_1_4)
        return false;

    if (!firstNodeSelected || !secondNodeSelected)
    {
        missionFeedback = "Select both escape route rooms first!";
        feedbackClock.restart();
        return false;
    }

    if (firstNode == secondNode)
    {
        missionFeedback = "Choose two different rooms!";
        feedbackClock.restart();
        return false;
    }

    if (firstNode != missionTargetA ||
        secondNode != missionTargetB)
    {
        missionFeedback = "Select the two assigned escape route rooms!";
        feedbackClock.restart();
        return false;
    }

    TreeNode* actualLCA = lcaDisplayTree->lca(
        missionTargetA,
        missionTargetB
    );

    if (actualLCA == nullptr)
        return false;

    expectedLCA = actualLCA->id;

    clearExpectedPath();
    buildExpectedPath();

    // Stop selecting the two target rooms.
    toggleLCAMode();

    lcaSelectionPhase = true;
    lcaPhaseComplete = false;
    pathPhaseActive = false;

    missionFeedback = "Now click the room where the routes meet!";
    feedbackClock.restart();

    return true;
}

bool Act1::isLCAPhaseComplete() const
{
    return lcaPhaseComplete;
}

void Act1::buildExpectedPath()
{
    clearExpectedPath();

    List* pathA = lcaDisplayTree->pathto(
        findNode(lcaDisplayTree->getTree(), missionTargetA)
    );

    List* pathB = lcaDisplayTree->pathto(
        findNode(lcaDisplayTree->getTree(), missionTargetB)
    );

    // Count both paths.
    int countA = 0;
    int countB = 0;

    for (List* curr = pathA; curr != nullptr; curr = curr->next)
        countA++;

    for (List* curr = pathB; curr != nullptr; curr = curr->next)
        countB++;

    // Copy paths into arrays.
    int* arrA = new int[countA];
    int* arrB = new int[countB];

    int i = 0;
    for (List* curr = pathA; curr != nullptr; curr = curr->next)
        arrA[i++] = curr->id;

    i = 0;
    for (List* curr = pathB; curr != nullptr; curr = curr->next)
        arrB[i++] = curr->id;

    // Find the common prefix.
    int common = 0;

    while (common < countA &&
        common < countB &&
        arrA[common] == arrB[common])
    {
        common++;
    }

    List* tail = nullptr;

    for (int j = countA - 1; j >= common - 1; j--)
    {
        List* node = new List(arrA[j], arrA[j]);

        if (expectedPath == nullptr)
            expectedPath = node;
        else
            tail->next = node;

        tail = node;
    }

    // Continue downward from the LCA's child to B.
    for (int j = common; j < countB; j++)
    {
        List* node = new List(arrB[j], arrB[j]);
        node->next = nullptr;

        if (expectedPath == nullptr)
            expectedPath = node;
        else
        {
          
            while (tail->next != nullptr)
                tail = tail->next;

            tail->next = node;
            tail = node;
        }
    }

    // Free temporary paths and arrays.
    while (pathA != nullptr)
    {
        List* temp = pathA;
        pathA = pathA->next;
        delete temp;
    }

    while (pathB != nullptr)
    {
        List* temp = pathB;
        pathB = pathB->next;
        delete temp;
    }

    delete[] arrA;
    delete[] arrB;
}

void Act1::recordPathClick(int id)
{
if(!missionActive || currentMission != Missions::MISSION_1_4 || !pathPhaseActive) return;
missionActionCount++;

    if (playerPathCount >= playerPathCapacity)
    {
        playerPathCapacity *= 2;

        int* newPath = new int[playerPathCapacity];

        for (int i = 0; i < playerPathCount; i++)
            newPath[i] = playerPath[i];

        delete[] playerPath;
        playerPath = newPath;
    }

    playerPath[playerPathCount++] = id;
}

bool Act1::checkPlayerPath()
{
    int expectedCount = 0;

    for (List* curr = expectedPath;
        curr != nullptr;
        curr = curr->next)
    {
        expectedCount++;
    }

    if (playerPathCount != expectedCount)
        return false;

    List* curr = expectedPath;

    for (int i = 0; i < playerPathCount; i++)
    {
        if (playerPath[i] != curr->id)
            return false;

        curr = curr->next;
    }

    return true;
}

bool Act1::submitPlayerPath()
{
    if (!missionActive ||
        currentMission != Missions::MISSION_1_4 ||
        !pathPhaseActive)
        return false;

    if (!checkPlayerPath())
    {
        missionFeedback = "Incorrect path! Try again.";
        feedbackClock.restart();

        playerPathCount = 0;
        return false;
    }

    missionFeedback = "Escape route confirmed! Mission Complete!";
    feedbackClock.restart();

    pathPhaseActive = false;
    completeMission();

    return true;
}

bool Act1::submitMeetingRoom(int clickedNode)
{
    if (!missionActive ||
        currentMission != Missions::MISSION_1_4 ||
        !lcaSelectionPhase)
        return false;

        missionActionCount++;

    if (clickedNode != expectedLCA)
    {
        missionFeedback = "Wrong meeting room! Try again.";
        feedbackClock.restart();
        return false;
    }

    lcaNode = findNode(tree->getTree(), clickedNode);
    lcaFound = true;

    lcaSelectionPhase = false;
    lcaPhaseComplete = true;
    pathPhaseActive = true;

    playerPathCount = 0;

    missionFeedback = "Correct meeting room! Now trace the escape path.";
    feedbackClock.restart();

    return true;
}

void Act1::prepareMirrorChallenge()
{
    if (tree == mirrorPlayerTree)
        tree = nullptr;

    delete mirrorTargetTree;
    delete mirrorPlayerTree;

    mirrorTargetTree = nullptr;
    mirrorPlayerTree = nullptr;

    mirrorTargetTree = new BinaryTree();
    mirrorPlayerTree = new BinaryTree();

    unsigned int seed = SeededRandom::deriveSeed(
        levelSeed, 5, 1
    );

    TreeNode* root = Act1LevelGenerator::generate(seed);

    mirrorTargetTree->setTree(root);
    mirrorPlayerTree->setTree(cloneTree(root));

    tree = mirrorPlayerTree;

    mirrorAutoMode = false;
    selectedNode = -1;
    nodeSelected = false;
}

void Act1::swapSelectedChildren(int nodeID)
{
    TreeNode* node = findNode(tree->getTree(), nodeID);

    if (node == nullptr)
        return;

    if (node->left == nullptr && node->right == nullptr)
        return;

    TreeNode* temp = node->left;
    node->left = node->right;
    node->right = temp;

    if (missionActive &&
        currentMission == Missions::MISSION_1_5)
    {
        missionActionCount++;
    }

    missionFeedback = "Children swapped!";
    feedbackClock.restart();
}

void Act1::swapMirrorNode()
{
    if (!nodeSelected)
        return;

    swapSelectedChildren(selectedNode);
}

bool Act1::compareMirrorTrees(TreeNode* original, TreeNode* player)
{
    if (original == nullptr && player == nullptr)
        return true;

    if (original == nullptr || player == nullptr)
        return false;

    if (original->id != player->id || original->val != player->val)
        return false;

    return compareMirrorTrees(original->left, player->right) &&
        compareMirrorTrees(original->right, player->left);
}

bool Act1::checkMirrorAnswer()
{
    if (mirrorTargetTree == nullptr || mirrorPlayerTree == nullptr)
        return false;

    return compareMirrorTrees(
        mirrorTargetTree->getTree(),
        mirrorPlayerTree->getTree()
    );
}

bool Act1::submitMirrorAnswer()
{
    if (!missionActive || currentMission != Missions::MISSION_1_5)
        return false;

    if (checkMirrorAnswer())
    {
        missionFeedback = "Perfect! The vault has been mirrored!";
        feedbackClock.restart();

        completeMission();
        return true;
    }

    missionFeedback = "Not quite! Keep mirroring the tree.";
    feedbackClock.restart();

    return false;
}

void Act1::autoMirror()
{
    if (!missionActive ||
        currentMission != Missions::MISSION_1_5)
        return;

    if (mirrorPlayerTree == nullptr)
        return;

    missionActionCount += 2 * countInternalNodes(
        mirrorPlayerTree->getTree()
    );

    mirrorPlayerTree->mirror();

    missionFeedback = "Auto mirror activated!";
    feedbackClock.restart();
}

int Act1::getMissionActionCount() const
{
    return missionActionCount;
}

void Act1::resetMissionActionCount()
{
    missionActionCount = 0;
}

int Act1::countInternalNodes(TreeNode* node)
{
    if (node == nullptr)
        return 0;

    if (node->left == nullptr && node->right == nullptr)
        return 0;

    return 1 +
        countInternalNodes(node->left) +
        countInternalNodes(node->right);
}

int Act1::getOptimalMissionSteps()
{
    int optimalSteps = 1;

    if (currentMission == Missions::MISSION_1_1)
    {
        int nodeCount = countNodes(tree->getTree());

        // Four traversal challenges.
        optimalSteps = 4 * nodeCount;
    }
    else if (currentMission == Missions::MISSION_1_2)
    {
        // First reconstruction round.
        int firstRoundNodes = tokenCount;

        // Generate the second round using its actual deterministic seed.
        unsigned int secondSeed = SeededRandom::deriveSeed(
            levelSeed, 2, 2
        );

        TreeNode* secondRoot = Act1LevelGenerator::generate(secondSeed);

        int secondRoundNodes = countNodes(secondRoot);

        Act1LevelGenerator::destroyTree(secondRoot);

        // Every token must be placed in both rounds.
        optimalSteps = firstRoundNodes + secondRoundNodes;
    }
    else if (currentMission == Missions::MISSION_1_3)
    {
        // Selecting each node + submitting its height.
        // Then one additional diameter submission.
        optimalSteps = (2 * heightNodeCount) + 1;
    }
    else if (currentMission == Missions::MISSION_1_4)
    {
        int pathCount = 0;

        for (List* curr = expectedPath;
            curr != nullptr;
            curr = curr->next)
        {
            pathCount++;
        }

        // Two target selections + meeting room + complete path.
        optimalSteps = 3 + pathCount;
    }
    else if (currentMission == Missions::MISSION_1_5)
    {
        optimalSteps = 2 * countInternalNodes(
            mirrorTargetTree->getTree()
        );
    }

    return optimalSteps;
}

int Act1::getNodeValueByID(TreeNode* node, int id)
{
    if (node == nullptr)
        return -1;

    if (node->id == id)
        return node->val;

    int leftResult = getNodeValueByID(node->left, id);

    if (leftResult != -1)
        return leftResult;

    return getNodeValueByID(node->right, id);
}