#include "TreeMirror.h"

VNode::VNode() : alive(false), id(0), key(0), left(0), right(0), parent(0), height(1), size(1),
x(0), y(0), dx(0), dy(0), placed(false), mark(VMark::NONE), cmpResult(0), alert(false) {
}

TreeMirror::TreeMirror() {
	cap = 64;
	nodes = new VNode[cap];
	rootId = 0; liveCount = 0; pendingKey = 0; curId = 0; layoutW = 0; layoutD = 0;
}

TreeMirror::~TreeMirror() { delete[] nodes; nodes = nullptr; }

void TreeMirror::ensure(int id) {
	if (id < cap) return;
	int nc = cap;
	while (nc <= id) nc *= 2;
	VNode* t = new VNode[nc];
	for (int i = 0; i < cap; i++) t[i] = nodes[i];
	delete[] nodes;
	nodes = t;
	cap = nc;
}

void TreeMirror::clear() {
	for (int i = 0; i < cap; i++) nodes[i] = VNode();
	rootId = 0; liveCount = 0; curId = 0; layoutW = 0; layoutD = 0;
}

void TreeMirror::clearMarks() {
	for (int i = 1; i < cap; i++) if (nodes[i].alive) { nodes[i].mark = VMark::NONE; nodes[i].cmpResult = 0; }
	curId = 0;
}

int TreeMirror::buildRec(TreeNode* n, int parent) {
	if (!n) return 0;
	int id = n->id;
	ensure(id);
	nodes[id] = VNode();
	nodes[id].alive = true; nodes[id].id = id; nodes[id].key = n->val; nodes[id].parent = parent;
	liveCount++;
	int l = buildRec(n->left, id);     // do not hold references across calls
	int r = buildRec(n->right, id);
	nodes[id].left = l; nodes[id].right = r;
	return id;
}

int TreeMirror::recomputeRec(int id) {
	if (!id) return 0;
	int l = nodes[id].left, r = nodes[id].right;
	int hl = recomputeRec(l), hr = recomputeRec(r);
	nodes[id].height = 1 + (hl > hr ? hl : hr);
	nodes[id].size = 1 + (l ? nodes[l].size : 0) + (r ? nodes[r].size : 0);
	return nodes[id].height;
}

void TreeMirror::layoutRec(int id, int depth, int& counter) {
	if (!id) return;
	layoutRec(nodes[id].left, depth + 1, counter);
	nodes[id].x = (float)counter++;
	nodes[id].y = (float)depth;
	if (depth + 1 > layoutD) layoutD = depth + 1;
	layoutRec(nodes[id].right, depth + 1, counter);
}

void TreeMirror::layout() {
	layoutD = 0;
	int counter = 0;
	layoutRec(rootId, 0, counter);
	layoutW = counter;
}

void TreeMirror::loadFromModel(TreeNode* modelRoot) {
	clear();
	rootId = buildRec(modelRoot, 0);
	recomputeRec(rootId);
	layout();
	for (int i = 1; i < cap; i++)
		if (nodes[i].alive) { nodes[i].dx = nodes[i].x; nodes[i].dy = nodes[i].y; nodes[i].placed = true; }
}

void TreeMirror::animateStep(float dt) {
	float f = dt * 10.0f;
	if (f > 1.0f) f = 1.0f;
	for (int i = 1; i < cap; i++) {
		VNode& n = nodes[i];
		if (!n.alive) continue;
		if (!n.placed) {                 // new nodes grow out of their parent
			if (isAlive(n.parent)) { n.dx = nodes[n.parent].dx; n.dy = nodes[n.parent].dy; }
			else { n.dx = n.x; n.dy = n.y; }
			n.placed = true;
		}
		n.dx += (n.x - n.dx) * f;
		n.dy += (n.y - n.dy) * f;
	}
}

int TreeMirror::balanceOf(int id) const {
	if (!isAlive(id)) return 0;
	return heightOf(nodes[id].left) - heightOf(nodes[id].right);
}

void TreeMirror::setCurrent(int id) {
	if (isAlive(curId) && nodes[curId].mark == VMark::CURRENT) nodes[curId].mark = VMark::VISITED;
	if (isAlive(id)) nodes[id].mark = VMark::CURRENT;
	curId = id;
}

void TreeMirror::replaceChild(int parent, int oldChild, int newChild) {
	if (parent == 0) rootId = newChild;
	else if (nodes[parent].left == oldChild) nodes[parent].left = newChild;
	else nodes[parent].right = newChild;
}

void TreeMirror::rotateRightAt(int z) {
	int y = nodes[z].left;
	if (!y) return;
	int t2 = nodes[y].right, p = nodes[z].parent;
	replaceChild(p, z, y);
	nodes[y].parent = p;
	nodes[y].right = z;  nodes[z].parent = y;
	nodes[z].left = t2;  if (t2) nodes[t2].parent = z;
}

void TreeMirror::rotateLeftAt(int z) {
	int y = nodes[z].right;
	if (!y) return;
	int t2 = nodes[y].left, p = nodes[z].parent;
	replaceChild(p, z, y);
	nodes[y].parent = p;
	nodes[y].left = z;   nodes[z].parent = y;
	nodes[z].right = t2; if (t2) nodes[t2].parent = z;
}

void TreeMirror::markSubtree(int id, int mark) {
	if (!isAlive(id)) return;
	nodes[id].mark = mark;
	markSubtree(nodes[id].left, mark);
	markSubtree(nodes[id].right, mark);
}

// Payloads
void TreeMirror::apply(const Events& e) {
	switch (e.type) {
	case EventTypes::VISIT:
		if (isAlive(e.id)) setCurrent(e.id);
		break;
	case EventTypes::COMPARE:
		if (isAlive(e.id)) { setCurrent(e.id); nodes[e.id].cmpResult = e.data; }
		break;
	case EventTypes::INSERT: {
		int id = e.id, parent = e.extra, side = e.data;
		ensure(id);
		nodes[id] = VNode();
		nodes[id].alive = true; nodes[id].id = id; nodes[id].key = pendingKey; nodes[id].parent = parent;
		if (parent == 0) rootId = id;
		else if (side == 1) nodes[parent].left = id;
		else nodes[parent].right = id;
		liveCount++;
		setCurrent(id);
		nodes[id].mark = VMark::NEWNODE;
		layout();
		break;
	}
	case EventTypes::UNLINK: {
		int id = e.id;
		if (!isAlive(id)) break;
		int c = nodes[id].left ? nodes[id].left : nodes[id].right;   // at most one child here
		int p = nodes[id].parent;
		replaceChild(p, id, c);
		if (c) nodes[c].parent = p;
		nodes[id].alive = false;
		liveCount--;
		if (curId == id) curId = 0;
		layout();
		break;
	}
	case EventTypes::SWAP_VALUE:
		if (isAlive(e.id) && isAlive(e.extra)) {
			setCurrent(e.extra);                 // successor
			nodes[e.id].key = e.data;
			nodes[e.id].mark = VMark::RESULT;
		}
		break;
	case EventTypes::ROTATE: {
		int pivot = e.id;
		if (!isAlive(pivot)) break;
		nodes[pivot].alert = false;
		if (e.extra == RotationType::LL) rotateRightAt(pivot);
		else if (e.extra == RotationType::RR) rotateLeftAt(pivot);
		else if (e.extra == RotationType::LR) { rotateLeftAt(nodes[pivot].left); rotateRightAt(pivot); }
		else if (e.extra == RotationType::RL) { rotateRightAt(nodes[pivot].right); rotateLeftAt(pivot); }
		layout();
		break;
	}
	case EventTypes::UPDATE_HEIGHT:
		if (isAlive(e.id)) nodes[e.id].height = e.data;
		break;
	case EventTypes::UPDATE_SIZE:
		if (isAlive(e.id)) nodes[e.id].size = e.data;
		break;
	case EventTypes::BALANCE_ALERT:
		if (isAlive(e.id)) nodes[e.id].alert = true;
		break;
	case EventTypes::HIGHLIGHT_SUBTREE:
		if (e.extra == 1) markSubtree(e.id, VMark::SKIPPED);
		else if (isAlive(e.id)) nodes[e.id].mark = (e.extra == 2) ? VMark::RESULT : VMark::VISITED;
		break;
	default:
		break;   // DONE etc.
	}
}

bool TreeMirror::matchRec(int id, TreeNode* n, int parent) const {
	if (!n) return id == 0;
	if (id == 0 || !isAlive(id)) return false;
	if (nodes[id].id != n->id || nodes[id].key != n->val || nodes[id].parent != parent) return false;
	return matchRec(nodes[id].left, n->left, id) && matchRec(nodes[id].right, n->right, id);
}

bool TreeMirror::matchesModel(TreeNode* modelRoot) const {
	return matchRec(rootId, modelRoot, 0);
}

int TreeMirror::annotRec(int id, int& height, int& bad) const {
	if (!id) { height = 0; return 0; }
	int hl, hr;
	int sl = annotRec(nodes[id].left, hl, bad);
	int sr = annotRec(nodes[id].right, hr, bad);
	height = 1 + (hl > hr ? hl : hr);
	int size = 1 + sl + sr;
	if (!bad && (nodes[id].size != size || nodes[id].height != height)) bad = id;
	return size;
}

int TreeMirror::firstBadAnnotation() const {
	int h = 0, bad = 0;
	annotRec(rootId, h, bad);
	return bad;
}