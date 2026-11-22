#pragma once

#include <SFML/Graphics.hpp>
#include "BinaryTree.h"
#include "Missions.h"
using namespace sf;

struct NodePosition
{
    int id;
    sf::Vector2f position;
};

struct Token
{
    int value;
    sf::Vector2f position;
    bool placed;
    int nodeID;
};


class Act1
{
private:
    BinaryTree* tree;

    float nodeRadius = 25.f;
    float levelSpacing = 100.f;
    float horizontalSpacing = 100.f;
    NodePosition* positions;
    int positionCount;
    int capacity;
    Font font;
    EventLog* log;
    int currentEvent;
    float eventTimer;
    float eventDelay;
    bool playing;
    sf::View treeView;
    sf::Vector2f lastMousePosition;
    bool dragging;
    float zoomLevel;
    int selectedNode;
    bool nodeSelected;
    std::string currentTraversal;
    List* currentPath;
    int firstNode;
    int secondNode;
    bool firstNodeSelected;
    bool secondNodeSelected;
    bool lcaMode;
    TreeNode* lcaNode;
    bool lcaFound;
    bool symmetryChecked;
    bool symmetric;
    bool comparisonDone;
    bool treesMatch;
    bool inputActive;
    std::string inputBuffer;
    std::string inputPrompt;
    int inputStep;
    int inputID;
    int inputValue;
    int inputParentID;
    bool inputIsLeft;
    int currentMission;
    bool missionActive;
    bool missionComplete;
    int traversalChallenge;

    List* expectedTraversal;

    int* playerSequence;
    int playerCount;
    int playerCapacity;
    std::string missionFeedback;
    sf::Clock feedbackClock;
    BinaryTree* targetTree;
    BinaryTree* playerTree;
    int reconstructionRound;
    BinaryTree* sandboxTree;
    Token* tokens;
    int tokenCount;

    int draggingToken;
    sf::Vector2f dragOffset;
    List* firstClue;
    List* secondClue;
    TreeNode* heightTarget;
    int* playerHeights;
    int heightNodeCount;
    int currentHeightIndex;
    int expectedDiameter;
    int playerDiameter;
    int* heightNodeIDs;
    BinaryTree* heightDisplayTree;
    int selectedHeightNode;
    std::string heightInput;
    bool heightInputActive;
    bool diameterInputActive;
    int missionTargetA;
    int missionTargetB;
    int expectedLCA;

    bool lcaPhaseComplete;
    bool pathPhaseActive;
    BinaryTree* lcaDisplayTree;
    List* expectedPath;
    int* playerPath;
    int playerPathCount;
    int playerPathCapacity;
    bool lcaSelectionPhase;
    BinaryTree* mirrorTargetTree;
    BinaryTree* mirrorPlayerTree;
    bool mirrorAutoMode;
    int missionActionCount = 0;

    unsigned int levelSeed;

    Vector2f getPosition(int id);
    void addPosition(int id, Vector2f position);
    void calculatePositions(TreeNode* node, int depth, float& nextX);
    void drawEdges(RenderWindow& window, TreeNode* node);
    void drawNodes(RenderWindow& window, TreeNode* node);
    Color getNodeColor(int id);
    void pause();
    void resume();
    void drawTraversalGuide(sf::RenderWindow& window);
    string getSelectedNodeInfo();
    void drawSelectedNodeInfo(sf::RenderWindow& window);
    void drawCurrentTraversal(sf::RenderWindow& window);
    void drawTreeStats(sf::RenderWindow& window);
    void clearCurrentPath();
    bool idExists(TreeNode* node, int id);
    void drawInput(sf::RenderWindow& window);
    void drawMissionUI(sf::RenderWindow& window);
    TreeNode* cloneBlueprint(TreeNode* node);
    void resetPlayerTree(TreeNode* node);
    void buildExpectedPath();
    void prepareMirrorChallenge();
    bool checkMirrorAnswer();
    void swapSelectedChildren(int nodeID);
    bool compareMirrorTrees(TreeNode* original, TreeNode* player);
    int countInternalNodes(TreeNode* node);
    int getNodeValueByID(TreeNode* node, int id);
public:
    Act1(BinaryTree* t, unsigned int seed);
    ~Act1();
    void setEventLog(EventLog* l);
    void update(float deltaTime);
    void draw(RenderWindow& window);
    void nextEvent();
    void previousEvent();
    void restart();
    void setSpeed(float delay);
    void togglePlayback();
    string getEventDescription();
    void drawEventInfo(RenderWindow& window);
    void handleMouseEvent(const sf::Event& event, sf::RenderWindow& window);
    void startDragging(sf::Vector2f mousePosition);
    void stopDragging();
    void dragView(sf::Vector2f mousePosition);
    void zoom(float delta);
    void resetCamera();
    void selectNode(sf::Vector2f mousePosition, sf::RenderWindow& window);
    TreeNode* findNode(TreeNode* node, int id);
    void runTraversal(int type);
    void mirrorTree();
    void showPathToSelectedNode();
    bool isInCurrentPath(int id);
    void toggleLCAMode();
    void findLCA();
    void drawLCAInfo(sf::RenderWindow& window);
    void checkSymmetry();
    void drawSymmetryInfo(sf::RenderWindow& window);
    void compareTree(TreeNode* otherRoot);
    void drawComparisonInfo(sf::RenderWindow& window);
    void createNewTree();
    bool createRoot(int id, int val);
    bool addNode(int parentID, int id, int val, bool isLeft);
    void handleTextInput(char32_t character);
    void startRootInput();
    void startChildInput();
    void finishInput();
    void cancelInput();
    void handleInputKey(sf::Keyboard::Key key);
    void setMission(int mission);
    int getMission() const;
    void startMission();
    bool isMissionActive() const;
    void completeMission();
    bool isMissionComplete() const;
    void resetMission();
    void nextMission();
    void prepareTraversalChallenge();
    void recordNodeClick(int id);
    bool checkTraversalAnswer();
    bool submitTraversalAnswer();
    void prepareReconstructionChallenge();
    void collectTokenValues(TreeNode* node, int& index);
    void createTokens();
    int countNodes(TreeNode* node);
    void drawTokens(sf::RenderWindow& window);
    void createSampleTarget();
    void createPlayerTree();
    void startTokenDrag(sf::Vector2f mousePosition);
    void updateTokenDrag(sf::Vector2f mousePosition);
    void dropToken(sf::Vector2f mousePosition, sf::RenderWindow& window);
    bool allTokensPlaced();
    bool compareReconstruction(TreeNode* target, TreeNode* player);
    bool submitReconstruction();
    void prepareReconstructionClues();
    void collectPostorderNodes(TreeNode* node, int* ids, int& index);
    void prepareHeightChallenge();
    void submitNodeHeight(int height);
    bool checkHeightAnswers();
    TreeNode* cloneTree(TreeNode* node);
    void handleHeightTextInput(char character);
    void submitHeightInput();
    void prepareLCAChallenge();
    void clearExpectedPath();
    bool submitLCAAnswer();
    bool isLCAPhaseComplete() const;
    void recordPathClick(int id);
    bool checkPlayerPath();
    bool submitPlayerPath();
    bool submitMeetingRoom(int clickedNode);
    void swapMirrorNode();
    bool submitMirrorAnswer();
    void autoMirror();
    int getPlayerCount() const;
    int getMissionActionCount() const;
    void resetMissionActionCount();
    int getOptimalMissionSteps();
};