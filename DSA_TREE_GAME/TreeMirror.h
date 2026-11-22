#pragma once
#include "BinaryTree.h"

struct VMark {
	static const int NONE = 0, CURRENT = 1, VISITED = 2, SKIPPED = 3, RESULT = 4, NEWNODE = 5;
};

struct VNode {
	bool alive;
	int id, key;
	int left, right, parent;     // node ids, 0 = none
	int height, size;            // as last reported by the model 
	float x, y;
	float dx, dy;
	bool placed;
	int mark;
	int cmpResult;               // last COMPARE result at this node: -1, 0, 1
	bool alert;                  // BALANCE_ALERT raised, not yet rotated away
	VNode();
};

class TreeMirror {
private:
	VNode* nodes;
	int cap;
	int rootId, liveCount, pendingKey, curId;
	int layoutW, layoutD;

	void ensure(int id);
	bool isAlive(int id) const { return id > 0 && id < cap && nodes[id].alive; }
	void setCurrent(int id);
	void replaceChild(int parent, int oldChild, int newChild);
	void rotateLeftAt(int id);
	void rotateRightAt(int id);
	void markSubtree(int id, int mark);
	int buildRec(TreeNode* n, int parent);
	int recomputeRec(int id);
	void layoutRec(int id, int depth, int& counter);
	bool matchRec(int id, TreeNode* n, int parent) const;
	int annotRec(int id, int& height, int& bad) const;
public:
	TreeMirror();
	~TreeMirror();
	TreeMirror(const TreeMirror&) = delete;
	TreeMirror& operator=(const TreeMirror&) = delete;

	void setPendingKey(int k) { pendingKey = k; }
	int getPendingKey() const { return pendingKey; }

	void clear();
	void loadFromModel(TreeNode* modelRoot);
	void apply(const Events& e);
	void clearMarks();
	void layout();
	void animateStep(float dt);

	const VNode* node(int id) const { return isAlive(id) ? &nodes[id] : nullptr; }
	int capacity() const { return cap; }
	int root() const { return rootId; }
	int count() const { return liveCount; }
	int layoutWidth() const { return layoutW; }
	int layoutDepth() const { return layoutD; }
	int keyOf(int id) const { return isAlive(id) ? nodes[id].key : 0; }
	int heightOf(int id) const { return isAlive(id) ? nodes[id].height : 0; }
	int balanceOf(int id) const;               // height(left) - height(right)

	int firstBadAnnotation() const;

	bool matchesModel(TreeNode* modelRoot) const;   // structure + keys + parent links
};