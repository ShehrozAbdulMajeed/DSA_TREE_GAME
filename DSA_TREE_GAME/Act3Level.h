#pragma once
#include "AVL.h"
#include "LevelData.h"

struct Act3Case { static const int LL = 0, RR = 1, LR = 2, RL = 3, NONE = 4; };

class Act3Level {
public:
	static const int PLAYING = 0, WON = 1, LOST = 2;
	static const int PH_DELIVERY = 0, PH_PLAY = 1;

	LevelDef def;
	AVL tree;
	int state, alarm, capacity, optimal, penalties, penaltyPoints, bonus;
	bool hintUsed;
	char message[128];
	int lastInsertKey, hintId, nodeCount, round;
	bool pendingReload;

	// 3.1 badges
	int unbalancedTotal, found;
	// 3.2 / 3.3 manual repair
	int caseId, moveCount, moveStep;
	bool identified;
	// 3.4 prediction
	int pendingKey, expected;
	// 3.5 order statistics
	int phase, needIdx, kRemain, curId, targetKey, kWanted, delivered, deliveryRotations;
	// endless
	float survived, opTimer;
	int opsApplied;

private:
	Rng rng;
	bool simulating;
	bool clicked[256];
	int mvId[2];
	bool mvLeft[2];
	int deliverKeys[64];
	bool botHasNext; int botNextId; bool botNextLeft;   // endless bot: second half of a double rotation

	void beginImpl(const LevelDef& d, EventLog* log, bool sim);
	void settle();
	void penalize(int alarmAdd, int points);
	void buildBalanced(AVL& t, int n);
	void buildBadges();
	void setupViolationRound();
	void pickPredictKey();
	void applyOp(EventLog* log);
	void checkInvalid();
	void enterOrder(TreeNode* n, EventLog* log);
	static void collectUnbalanced(AVL& t, AVLNode* n, int* ids, int& cnt, int cap);
	static AVLNode* findById(TreeNode* n, int id);
	static AVLNode* lowestUnbalanced(AVL& t);
	static int classify(AVL& t, AVLNode* z);
	static void keysOf(AVL& t, int* out, int& n, int cap);
	void movesFor(AVLNode* z, int c);
	int orderDir();
public:
	Act3Level();
	void begin(const LevelDef& d, EventLog* log = nullptr);

	// queries for the UI
	int unbalancedCount();
	int unbalancedIds(int* ids, int cap);
	int lowestUnbalancedId();
	static const char* caseName(int c);

	// 3.1
	bool clickNode(int id, EventLog* log);
	// 3.2 / 3.3 / endless: rotate the node with this id (left = rotate left)
	bool rotate(int id, bool left, EventLog* log);
	// 3.3: identify the case (Act3Case::LL..RL)
	bool answerCase(int c, EventLog* log);
	// 3.4: predict (Act3Case::LL..NONE); the insert then runs either way
	bool predict(int c, EventLog* log);
	// 3.5: dir 1 = left, 2 = right
	bool move(int dir, EventLog* log);
	void deliverNext(EventLog* log);
	int correctDir();
	bool tick(float dt, EventLog* log);

	void hint();
	bool autoStep(EventLog* log);       // one perfect action (optimal, tests)
	int stars() const;
	int score() const;
	static int simulate(const LevelDef& d);
};