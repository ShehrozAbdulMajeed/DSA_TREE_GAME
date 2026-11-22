#pragma once
#include "BinaryTree.h"

struct VMark {
	static const int NONE = 0, CURRENT = 1, VISITED = 2, SKIPPED = 3, RESULT = 4, NEWNODE = 5;
};

struct VNode {
	bool alive;
	int id, key;
	int left, right, parent;
	int height, size;
	float x, y;
	float dx, dy;
	bool placed;
	int mark;
	int cmpResult;
	bool alert;
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
public:
	TreeMirror();
	~TreeMirror();
	TreeMirror(const TreeMirror&) = delete;
	TreeMirror& operator=(const TreeMirror&) = delete;

	// INSERT events do not carry the key, so the caller sets it before replaying an insert log.
	void setPendingKey(int k) { pendingKey = k; }
	int getPendingKey() const { return pendingKey; }

	void clear();
	void loadFromModel(TreeNode* modelRoot);   // snapshot, no animation
	void apply(const Events& e);               // one event
	void clearMarks();
	void layout();
	void animateStep(float dt);                // eases dx,dy toward x,y

	const VNode* node(int id) const { return isAlive(id) ? &nodes[id] : nullptr; }
	int capacity() const { return cap; }
	int root() const { return rootId; }
	int count() const { return liveCount; }
	int layoutWidth() const { return layoutW; }
	int layoutDepth() const { return layoutD; }
	int keyOf(int id) const { return isAlive(id) ? nodes[id].key : 0; }
	int heightOf(int id) const { return isAlive(id) ? nodes[id].height : 0; }
	int balanceOf(int id) const;               // height(left) - height(right)

	bool matchesModel(TreeNode* modelRoot) const;   // structure + keys + parent links
};