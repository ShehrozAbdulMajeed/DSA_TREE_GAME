#include "Act2Level.h"
#include "TextUtil.h"

Act2Level::Act2Level() : rng(1) {
	state = PLAYING; phase = PH_PLAY; alarm = 0; capacity = 1000000; optimal = 0;
	penalties = 0; penaltyPoints = 0; bonus = 0; hintUsed = false; message[0] = 0;
	lastInsertKey = 0; hintId = 0; nodeCount = 0; curId = 0; targetKey = 0; round = 0; targetId = 0;
	lo = hi = 0; inRangeTotal = collected = 0; delivered = 0; simulating = false; shrinkTimer = 0;
	for (int i = 0; i < 256; i++) { entered[i] = false; pruned[i] = false; }
}

BSTNode* Act2Level::findById(TreeNode* n, int id) {
	if (!n) return nullptr;
	if (n->id == id) return static_cast<BSTNode*>(n);
	BSTNode* l = findById(n->left, id);
	return l ? l : findById(n->right, id);
}

int Act2Level::caseOf(BSTNode* n) {
	if (n->left && n->right) return 2;
	if (n->left || n->right) return 1;
	return 0;
}

BSTNode* Act2Level::parentOf(int id) {
	BSTNode* me = findById(tree.getTree(), id);
	if (!me) return nullptr;
	BSTNode* par = nullptr;
	TreeNode* cur = tree.getTree();
	while (cur && cur->id != id) { par = static_cast<BSTNode*>(cur); cur = (me->val < cur->val) ? cur->left : cur->right; }
	return par;
}

static void copyEvents(const EventLog& src, EventLog* dst, bool onlyInsert) {
	if (!dst) return;
	for (int i = 0; i < src.size(); i++) {
		const Events& e = src.get(i);
		if (e.type == EventTypes::DONE) continue;
		if (onlyInsert && e.type != EventTypes::INSERT) continue;
		dst->add(e.type, e.id, e.extra, e.data);
	}
}

void Act2Level::buildTree() {
	if (def.keyCount > 0) {
		for (int i = 0; i < def.keyCount; i++) tree.insert(def.keys[i]);
		return;
	}
	int n = def.nodes < 1 ? 1 : (def.nodes > 60 ? 60 : def.nodes);
	int maxSoFar = 0, placed = 0, guard = 0;
	while (placed < n && guard++ < 10000) {
		int k = 1 + rng.upto(99);
		if (def.skew > 0 && placed > 0 && rng.upto(100) < def.skew) {
			k = maxSoFar + 1 + rng.upto(4);
			if (k > 99) k = 1 + rng.upto(99);
		}
		if (tree.insert(k)) { placed++; if (k > maxSoFar) maxSoFar = k; }
	}
}

int Act2Level::randomExistingKey() {
	List* l = tree.inOrder_R();
	int keys[256]; int n = 0;
	for (List* c = l; c&& n < 256; c = c->next) keys[n++] = c->val;
	freeList(l);
	return n > 0 ? keys[rng.upto(n)] : 0;
}

int Act2Level::randomAbsentKey() {
	for (int g = 0; g < 1000; g++) {
		int k = 1 + rng.upto(99);
		if (!tree.search(k)) return k;
	}
	return 100;
}

void Act2Level::settle() {
	if (state == PLAYING && alarm >= capacity) {
		state = LOST;
		tCopy(message, 128, "The alarm is full - the guards are coming!");
	}
}

void Act2Level::penalize(int points) {
	alarm++; penalties++; penaltyPoints += points;
	settle();
}

void Act2Level::begin(const LevelDef& d, EventLog* log) {
	beginImpl(d, log, false);
}

void Act2Level::beginImpl(const LevelDef& d, EventLog* log, bool sim) {
	def = d; simulating = sim;
	rng = Rng((unsigned int)d.seed * 7919u + 13u);
	tree.reset();
	state = PLAYING; phase = PH_PLAY; alarm = 0; penalties = 0; penaltyPoints = 0; bonus = 0;
	hintUsed = false; message[0] = 0; lastInsertKey = 0; hintId = 0;
	curId = 0; targetKey = 0; round = 0; targetId = 0; lo = hi = 0; inRangeTotal = collected = 0; delivered = 0;
	shrinkTimer = 0; capacity = 1000000; optimal = 0;
	for (int i = 0; i < 256; i++) { entered[i] = false; pruned[i] = false; }

	switch (def.type) {
	case LevelType::SEARCH:
		buildTree();
		targetKey = def.target >= 0 ? def.target : randomExistingKey();
		enterNav(tree.getTree(), log);
		break;
	case LevelType::INSERT:
		buildTree();
		targetKey = def.target >= 0 ? def.target : randomAbsentKey();
		enterNav(tree.getTree(), log);
		break;
	case LevelType::REMOVE:
		buildTree();
		pickRemoveTarget();
		break;
	case LevelType::RANGE: {
		buildTree();
		List* l = tree.inOrder_R();
		int keys[64]; int n = 0;
		for (List* c = l; c && n < 64; c = c->next) keys[n++] = c->val;
		freeList(l);
		if (def.lo >= 0 && def.hi >= def.lo) { lo = def.lo; hi = def.hi; }
		else {
			int a = n / 4 + rng.upto(n / 4 + 1);
			int b = a + n / 3 + rng.upto(2);
			if (b >= n) b = n - 1;
			if (a > b) a = b;
			lo = keys[a]; hi = keys[b];
		}
		for (int i = 0; i < n; i++) if (keys[i] >= lo && keys[i] <= hi) inRangeTotal++;
		enterRange(tree.getTree(), log);
		break;
	}
	case LevelType::BOSS:
		phase = PH_DELIVERY;
		targetKey = def.nodes;
		break;
	}

	// nodeCount for the pruning bonus
	List* all = tree.inOrder_R();
	nodeCount = 0;
	for (List* c = all; c; c = c->next) nodeCount++;
	freeList(all);
	if (def.type == LevelType::BOSS) nodeCount = def.nodes;

	if (!sim) {
		optimal = simulate(def);
		capacity = (3 * optimal + 1) / 2 + 2;
		settle();
	}
}

int Act2Level::simulate(const LevelDef& d) {
	Act2Level s;
	s.beginImpl(d, nullptr, true);
	int guard = 0;
	while (s.state == PLAYING && guard++ < 10000)
		if (!s.autoStep(nullptr)) break;
	return s.alarm;
}

// navigation (search / insert / boss) 

int Act2Level::correctDir() const {
	BSTNode* n = findById(const_cast<BST&>(tree).getTree(), curId);
	if (!n || n->val == targetKey) return 0;
	return targetKey < n->val ? 1 : 2;
}

void Act2Level::enterNav(TreeNode* n, EventLog* log) {
	if (!n) return;
	curId = n->id;
	alarm++;
	if (log) log->add(EventTypes::VISIT, n->id);
	if ((def.type == LevelType::SEARCH || def.type == LevelType::BOSS) && n->val == targetKey) {
		state = WON;
		if (log) log->add(EventTypes::HIGHLIGHT_SUBTREE, n->id, 2);
		tCopy(message, 128, "Target reached!");
	}
	if (log) log->add(EventTypes::DONE);
	settle();
}

bool Act2Level::move(int dir, EventLog* log) {
	if (state != PLAYING || phase != PH_PLAY) return false;
	if (def.type != LevelType::SEARCH && def.type != LevelType::INSERT && def.type != LevelType::BOSS) return false;
	BSTNode* n = findById(tree.getTree(), curId);
	if (!n) return false;
	int want = correctDir();
	if (want == 0) return false;
	if (dir != want) {
		tCopy(message, 128, "Wrong way (+1 alarm): ");
		tAppendInt(message, 128, targetKey);
		tAppend(message, 128, want == 1 ? " is smaller than " : " is larger than ");
		tAppendInt(message, 128, n->val);
		tAppend(message, 128, want == 1 ? ", so go left." : ", so go right.");
		penalize(0);
		return false;
	}
	TreeNode* child = (dir == 1) ? n->left : n->right;
	if (!child) {
		if (def.type != LevelType::INSERT) return false;
		EventLog tmp;
		tree.insert(targetKey, &tmp);
		lastInsertKey = targetKey;
		copyEvents(tmp, log, true);
		if (log) log->add(EventTypes::DONE);
		state = WON;
		tCopy(message, 128, "Stashed key ");
		tAppendInt(message, 128, targetKey);
		tAppend(message, 128, " at the right spot!");
		return true;
	}
	enterNav(child, log);
	return true;
}

bool Act2Level::clickNode(int id, EventLog* log) {
	if (state != PLAYING || id == 0) return false;
	if (def.type == LevelType::RANGE) return rangeClick(id, log);
	if (def.type == LevelType::REMOVE || phase != PH_PLAY) return false;
	BSTNode* n = findById(tree.getTree(), curId);
	if (!n) return false;
	if (n->left && n->left->id == id) return move(1, log);
	if (n->right && n->right->id == id) return move(2, log);
	tCopy(message, 128, "You can only move to a room next to you.");
	return false;
}

// remove 

void Act2Level::pickRemoveTarget() {
	int want = (round < def.rounds && round < 8) ? def.cases[round] : -1;
	List* l = tree.inOrder_R();
	int cand[256]; int cnt = 0;
	int any[256]; int anyCnt = 0;
	for (List* c = l; c && anyCnt < 256; c = c->next) {
		BSTNode* b = tree.search(c->val);
		if (!b) continue;
		any[anyCnt++] = c->val;
		if (want < 0 || caseOf(b) == want) cand[cnt++] = c->val;
	}
	freeList(l);
	int key;
	if (cnt > 0) key = cand[rng.upto(cnt)];
	else if (anyCnt > 0) key = any[rng.upto(anyCnt)];
	else { state = WON; return; }
	BSTNode* b = tree.search(key);
	targetId = b->id; targetKey = key;
	tCopy(message, 128, "Which case applies to key ");
	tAppendInt(message, 128, key);
	tAppend(message, 128, "?");
}

bool Act2Level::answerCase(int c, EventLog* log) {
	if (state != PLAYING || def.type != LevelType::REMOVE) return false;
	BSTNode* b = tree.search(targetKey);
	if (!b) return false;
	int right = caseOf(b);
	if (c != right) {
		tCopy(message, 128, "Not quite (+1 alarm): key ");
		tAppendInt(message, 128, targetKey);
		tAppend(message, 128, right == 0 ? " is a leaf." : (right == 1 ? " has exactly one child." : " has two children."));
		penalize(25);
		return false;
	}
	EventLog tmp;
	tree.remove(targetKey, &tmp);
	int compares = 0;
	for (int i = 0; i < tmp.size(); i++) if (tmp.get(i).type == EventTypes::COMPARE) compares++;
	copyEvents(tmp, log, false);
	if (log) log->add(EventTypes::DONE);
	alarm += compares;
	tCopy(message, 128, "Key ");
	tAppendInt(message, 128, targetKey);
	tAppend(message, 128, " removed.");
	round++;
	if (round >= def.rounds) state = WON;
	else pickRemoveTarget();
	settle();
	return true;
}

// range 

bool Act2Level::hasInRange(TreeNode* t) {
	if (!t) return false;
	if (t->val >= lo && t->val <= hi) return true;
	return hasInRange(t->left) || hasInRange(t->right);
}

int Act2Level::needed(TreeNode* t) {
	if (!t) return 0;
	if (t->id < 256 && entered[t->id]) {
		int r = needed(t->left);
		return r ? r : needed(t->right);
	}
	return hasInRange(t) ? t->id : 0;
}

void Act2Level::enterRange(TreeNode* t, EventLog* log) {
	if (!t) return;
	if (t->id < 256) entered[t->id] = true;
	alarm++;
	bool in = (t->val >= lo && t->val <= hi);
	if (in) collected++;
	if (log) log->add(EventTypes::HIGHLIGHT_SUBTREE, t->id, in ? 2 : 0);
	if (t->val < lo && t->left) {
		if (t->left->id < 256) pruned[t->left->id] = true;
		if (log) log->add(EventTypes::HIGHLIGHT_SUBTREE, t->left->id, 1);
	}
	if (t->val > hi && t->right) {
		if (t->right->id < 256) pruned[t->right->id] = true;
		if (log) log->add(EventTypes::HIGHLIGHT_SUBTREE, t->right->id, 1);
	}
	if (collected >= inRangeTotal) {
		state = WON;
		int b = 10 * (nodeCount - alarm);
		bonus = b > 0 ? b : 0;
		tCopy(message, 128, "Range cleared!");
	}
	if (log) log->add(EventTypes::DONE);
	settle();
}

bool Act2Level::rangeClick(int id, EventLog* log) {
	if (state != PLAYING || def.type != LevelType::RANGE || id <= 0 || id >= 256) return false;
	BSTNode* n = findById(tree.getTree(), id);
	if (!n) return false;
	if (entered[id]) { tCopy(message, 128, "You are already in that room."); return false; }
	BSTNode* p = parentOf(id);
	if (!p || !entered[p->id]) { tCopy(message, 128, "Enter its parent room first."); return false; }
	if (pruned[id]) {
		tCopy(message, 128, "Pruned room (+1 alarm): its keys can never be in the range.");
		penalize(0);
		return false;
	}
	enterRange(n, log);
	return true;
}

// boss 

void Act2Level::deliverNext(EventLog* log) {
	if (def.type != LevelType::BOSS || phase != PH_DELIVERY) return;
	int key = ++delivered;
	EventLog tmp;
	tree.insert(key, &tmp);
	lastInsertKey = key;
	copyEvents(tmp, log, false);
	if (log) log->add(EventTypes::DONE);
	if (delivered >= def.nodes) {
		phase = PH_PLAY;
		tCopy(message, 128, "All deliveries are in. Find the last treasure!");
		EventLog* none = nullptr;
		(void)none;
		enterNav(tree.getTree(), log);
	}
}

void Act2Level::tick(float dt) {
	if (def.type != LevelType::BOSS || phase != PH_PLAY || state != PLAYING || simulating) return;
	shrinkTimer += dt;
	int period = def.shrink > 0 ? def.shrink : 3;
	int floorCap = optimal + 1;
	while (shrinkTimer >= (float)period) {
		shrinkTimer -= (float)period;
		if (capacity > floorCap) capacity--;
	}
	settle();
}

// hints, bot, results 

int Act2Level::targetNodeId() {
	if (def.type == LevelType::REMOVE) return targetId;
	if ((def.type == LevelType::SEARCH || def.type == LevelType::BOSS) && phase == PH_PLAY) {
		BSTNode* b = tree.search(targetKey);
		return b ? b->id : 0;
	}
	return 0;
}

void Act2Level::hint() {
	if (state != PLAYING) return;
	hintUsed = true;
	hintId = 0;
	if (def.type == LevelType::REMOVE) {
		BSTNode* b = tree.search(targetKey);
		if (b) {
			tCopy(message, 128, "Hint: key ");
			tAppendInt(message, 128, targetKey);
			tAppend(message, 128, caseOf(b) == 0 ? " has no children." : (caseOf(b) == 1 ? " has one child." : " has two children."));
		}
	}
	else if (def.type == LevelType::RANGE) {
		hintId = needed(tree.getTree());
		tCopy(message, 128, "Hint: enter the highlighted room.");
	}
	else if (phase == PH_PLAY) {
		int d = correctDir();
		BSTNode* n = findById(tree.getTree(), curId);
		if (n && d) {
			TreeNode* c = (d == 1) ? n->left : n->right;
			hintId = c ? c->id : 0;
			tCopy(message, 128, d == 1 ? "Hint: go left." : "Hint: go right.");
			if (!c) tAppend(message, 128, " The new room goes into the empty spot.");
		}
	}
}

bool Act2Level::autoStep(EventLog* log) {
	if (state != PLAYING) return false;
	if (phase == PH_DELIVERY) { deliverNext(log); return true; }
	switch (def.type) {
	case LevelType::SEARCH: case LevelType::INSERT: case LevelType::BOSS:
		return move(correctDir(), log);
	case LevelType::REMOVE: {
		BSTNode* b = tree.search(targetKey);
		return b && answerCase(caseOf(b), log);
	}
	case LevelType::RANGE: {
		int id = needed(tree.getTree());
		return id != 0 && rangeClick(id, log);
	}
	}
	return false;
}

int Act2Level::stars() const {
	if (state != WON) return 0;
	int s;
	if (alarm <= optimal && penalties == 0) s = 3;
	else if (alarm * 4 <= optimal * 5 && penalties <= 1) s = 2;
	else s = 1;
	if (hintUsed && s > 2) s = 2;                                   // a hint caps the stars at 2
	return s;
}

int Act2Level::score() const {
	int v = 1000 - 10 * alarm - penaltyPoints + bonus;
	return v < 0 ? 0 : v;
}