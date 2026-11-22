#pragma once
#include "BST.h"
#include "LevelData.h"

class Act2Level {
public:
	static const int PLAYING = 0, WON = 1, LOST = 2;
	static const int PH_DELIVERY = 0, PH_PLAY = 1;

	LevelDef def;
	BST tree;
	int state, phase;
	int alarm, capacity, optimal, penalties, penaltyPoints, bonus;
	bool hintUsed;
	char message[128];
	int lastInsertKey;
	int hintId;
	int nodeCount;

	// search / insert / boss
	int curId, targetKey;
	// remove
	int round, targetId;
	// range
	int lo, hi, inRangeTotal, collected;
	// boss
	int delivered;

private:
	Rng rng;
	bool simulating;
	float shrinkTimer;
	bool entered[256];
	bool pruned[256];

	void beginImpl(const LevelDef& d, EventLog* log, bool sim);
	void buildTree();
	int randomExistingKey();
	int randomAbsentKey();
	void settle();
	void penalize(int points);
	void enterNav(TreeNode* n, EventLog* log);
	void enterRange(TreeNode* n, EventLog* log);
	void pickRemoveTarget();
	int needed(TreeNode* t);
	bool hasInRange(TreeNode* t);
	BSTNode* parentOf(int id);
	static BSTNode* findById(TreeNode* n, int id);
	static int caseOf(BSTNode* n);
public:
	Act2Level();

	void begin(const LevelDef& d, EventLog* log = nullptr);

	// navigation levels (search / insert / boss): dir 1 = left, 2 = right 
	bool move(int dir, EventLog* log);
	bool clickNode(int id, EventLog* log);
	int correctDir() const;

	// remove level: 0 leaf, 1 one child, 2 two children 
	bool answerCase(int c, EventLog* log);

	// range level 
	bool rangeClick(int id, EventLog* log);

	// --- boss ---
	void deliverNext(EventLog* log);
	void tick(float dt);

	void hint();
	bool autoStep(EventLog* log);
	int targetNodeId();

	int stars() const;
	int score() const;
	static int simulate(const LevelDef& d);
};