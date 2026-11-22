#include "Act3Level.h"
#include "StringUtil.h"
#include <cstring>


static const int MAX_BALANCE = 3;        // endless: any room with balance >= 3 = collapse

Act3Level::Act3Level() : rng(1) {
	state = PLAYING; alarm = 0; capacity = 1000000; optimal = 0; penalties = 0; penaltyPoints = 0; bonus = 0;
	hintUsed = false; message[0] = 0; lastInsertKey = 0; hintId = 0; nodeCount = 0; round = 0; pendingReload = false;
	unbalancedTotal = found = 0; caseId = 0; moveCount = moveStep = 0; identified = true;
	pendingKey = 0; expected = 0; phase = PH_PLAY; needIdx = kRemain = curId = targetKey = kWanted = 0; delivered = 0; deliveryRotations = 0;
	survived = 0; opTimer = 0; opsApplied = 0; simulating = false;
	for (int i = 0; i < 256; i++) clicked[i] = false;
	mvId[0] = mvId[1] = 0; mvLeft[0] = mvLeft[1] = false;
	for (int i = 0; i < 64; i++) deliverKeys[i] = 0;
	botHasNext = false; botNextId = 0; botNextLeft = false;
}

const char* Act3Level::caseName(int c) {
	switch (c) { case Act3Case::LL: return "LL"; case Act3Case::RR: return "RR"; case Act3Case::LR: return "LR"; case Act3Case::RL: return "RL"; }
								  return "no rotation";
}

// helpers 

AVLNode* Act3Level::findById(TreeNode* n, int id) {
	if (!n) return nullptr;
	if (n->id == id) return static_cast<AVLNode*>(n);
	AVLNode* l = findById(n->left, id);
	return l ? l : findById(n->right, id);
}

void Act3Level::collectUnbalanced(AVL& t, AVLNode* n, int* ids, int& cnt, int cap) {
	if (!n) return;
	collectUnbalanced(t, static_cast<AVLNode*>(n->left), ids, cnt, cap);
	int bf = t.balanceFactor(n);
	if ((bf > 1 || bf < -1) && cnt < cap) ids[cnt++] = n->id;
	collectUnbalanced(t, static_cast<AVLNode*>(n->right), ids, cnt, cap);
}

AVLNode* Act3Level::lowestUnbalanced(AVL& t) {
	int ids[256]; int cnt = 0;
	collectUnbalanced(t, static_cast<AVLNode*>(t.getTree()), ids, cnt, 256);
	AVLNode* best = nullptr;
	for (int i = 0; i < cnt; i++) {
		AVLNode* n = findById(t.getTree(), ids[i]);
		if (!best || n->height < best->height) best = n;
	}
	return best;
}

int Act3Level::classify(AVL& t, AVLNode* z) {
	int bf = t.balanceFactor(z);
	if (bf > 1) return t.balanceFactor(static_cast<AVLNode*>(z->left)) >= 0 ? Act3Case::LL : Act3Case::LR;
	return t.balanceFactor(static_cast<AVLNode*>(z->right)) <= 0 ? Act3Case::RR : Act3Case::RL;
}

void Act3Level::keysOf(AVL& t, int* out, int& n, int cap) {
	List* l = t.preOrder_R();
	n = 0;
	for (List* c = l; c && n < cap; c = c->next) out[n++] = c->val;
	freeList(l);
}

int Act3Level::unbalancedIds(int* ids, int cap) {
	int cnt = 0;
	collectUnbalanced(tree, static_cast<AVLNode*>(tree.getTree()), ids, cnt, cap);
	return cnt;
}
int Act3Level::unbalancedCount() { int ids[256]; return unbalancedIds(ids, 256); }
int Act3Level::lowestUnbalancedId() { AVLNode* z = lowestUnbalanced(tree); return z ? z->id : 0; }

void Act3Level::movesFor(AVLNode* z, int c) {
	moveStep = 0;
	if (c == Act3Case::LL) { moveCount = 1; mvId[0] = z->id; mvLeft[0] = false; }
	else if (c == Act3Case::RR) { moveCount = 1; mvId[0] = z->id; mvLeft[0] = true; }
	else if (c == Act3Case::LR) { moveCount = 2; mvId[0] = z->left->id; mvLeft[0] = true; mvId[1] = z->id; mvLeft[1] = false; }
	else { moveCount = 2; mvId[0] = z->right->id; mvLeft[0] = false; mvId[1] = z->id; mvLeft[1] = true; }
}

void Act3Level::buildBalanced(AVL& t, int n) {
	t.reset();
	t.balancing = true;
	n = n < 1 ? 1 : (n > 40 ? 40 : n);
	int placed = 0, guard = 0;
	while (placed < n && guard++ < 10000)
		if (t.insert(1 + rng.upto(99))) placed++;
}

void Act3Level::settle() {
	if (state == PLAYING && def.type != LevelType::ENDLESS && alarm >= capacity) {
		state = LOST;
		copyText(message, 128, "The alarm is full - the guards are coming!");
	}
}

void Act3Level::penalize(int alarmAdd, int points) {
	alarm += alarmAdd; penalties++; penaltyPoints += points;
	settle();
}

// begin 

void Act3Level::begin(const LevelDef& d, EventLog* log) { beginImpl(d, log, false); }

void Act3Level::buildBadges() {
	int n = def.nodes < 3 ? 3 : (def.nodes > 40 ? 40 : def.nodes);
	for (int attempt = 0; attempt < 200; attempt++) {
		tree.reset();
		tree.balancing = false;
		int maxSoFar = 0, placed = 0, guard = 0;
		while (placed < n && guard++ < 10000) {
			int k = 1 + rng.upto(99);
			if (def.skew > 0 && placed > 0 && rng.upto(100) < def.skew) { k = maxSoFar + 1 + rng.upto(4); if (k > 99) k = 1 + rng.upto(99); }
			if (tree.insert(k)) { placed++; if (k > maxSoFar) maxSoFar = k; }
		}
		if (unbalancedCount() >= 2) return;
	}
}

void Act3Level::setupViolationRound() {
	int want = def.cases[round < 8 ? round : 7];
	int n = def.nodes;
	int base[64]; int nb = 0, chosen = -1;
	bool exact = false;
	for (int attempt = 0; attempt < 60 && !exact; attempt++) {
		AVL b;
		buildBalanced(b, n);
		keysOf(b, base, nb, 64);
		int start = rng.upto(99);
		int firstAny = -1;
		for (int t = 0; t < 99; t++) {
			int k = (start + t) % 99 + 1;
			if (b.search(k)) continue;
			AVL c;
			c.balancing = false;
			for (int i = 0; i < nb; i++) c.insert(base[i]);
			c.insert(k);
			AVLNode* z = lowestUnbalanced(c);
			if (!z) continue;
			if (firstAny < 0) firstAny = k;
			if (classify(c, z) == want) { chosen = k; exact = true; break; }
		}
		if (!exact && chosen < 0 && firstAny >= 0 && attempt == 59) chosen = firstAny;   // fallback
	}
	tree.reset();
	tree.balancing = false;
	for (int i = 0; i < nb; i++) tree.insert(base[i]);
	if (chosen > 0) tree.insert(chosen);
	AVLNode* z = lowestUnbalanced(tree);
	if (z) { caseId = classify(tree, z); movesFor(z, caseId); }
	identified = (def.type == LevelType::SINGLE);
	pendingReload = true;
	copyText(message, 128, def.type == LevelType::DOUBLE ? "Identify the case, then rotate." : "Select the lowest unbalanced room and rotate it.");
}

void Act3Level::pickPredictKey() {
	int want = def.cases[round < 8 ? round : 7];
	int base[64]; int nb = 0;
	keysOf(tree, base, nb, 64);
	int start = rng.upto(99), pick = -1, pickOutcome = Act3Case::NONE;
	for (int t = 0; t < 99; t++) {
		int k = (start + t) % 99 + 1;
		if (tree.search(k)) continue;
		AVL c;
		c.balancing = false;
		for (int i = 0; i < nb; i++) c.insert(base[i]);
		c.balancing = true;
		EventLog tmp;
		c.insert(k, 0, &tmp);
		int outcome = Act3Case::NONE;
		for (int i = 0; i < tmp.size(); i++) if (tmp.get(i).type == EventTypes::ROTATE) { outcome = tmp.get(i).extra; break; }
		if (pick < 0) { pick = k; pickOutcome = outcome; }
		if (outcome == want) { pick = k; pickOutcome = outcome; break; }
	}
	pendingKey = pick; expected = pickOutcome;
	copyText(message, 128, "Predict what the vault will do when key ");
	appendInt(message, 128, pendingKey);
	appendText(message, 128, " is inserted.");
}

void Act3Level::beginImpl(const LevelDef& d, EventLog* log, bool sim) {
	def = d; simulating = sim;
	rng = Rng((unsigned int)d.seed * 7919u + 29u);
	tree.reset();
	state = PLAYING; alarm = 0; penalties = 0; penaltyPoints = 0; bonus = 0; hintUsed = false; message[0] = 0;
	lastInsertKey = 0; hintId = 0; round = 0; pendingReload = false;
	unbalancedTotal = found = 0; caseId = 0; moveCount = moveStep = 0; identified = true;
	pendingKey = 0; expected = 0; phase = PH_PLAY; needIdx = kRemain = curId = targetKey = kWanted = 0; delivered = 0; deliveryRotations = 0;
	survived = 0; opTimer = 0; opsApplied = 0;
	capacity = 1000000; optimal = 0;
	botHasNext = false;
	for (int i = 0; i < 256; i++) clicked[i] = false;

	switch (def.type) {
	case LevelType::BADGES:
		buildBadges();
		unbalancedTotal = unbalancedCount();
		copyText(message, 128, "Click every unbalanced (red) room.");
		break;
	case LevelType::SINGLE:
	case LevelType::DOUBLE:
		setupViolationRound();
		pendingReload = false;             // the screen loads the first tree itself
		break;
	case LevelType::PREDICT:
		buildBalanced(tree, def.nodes);
		pickPredictKey();
		break;
	case LevelType::ORDER: {
		tree.balancing = true;
		phase = PH_DELIVERY;
		int n = def.nodes < 3 ? 3 : (def.nodes > 60 ? 60 : def.nodes);
		for (int i = 0; i < n; i++) deliverKeys[i] = 10 * (i + 1) + rng.upto(7);
		copyText(message, 128, "Sorted deliveries are arriving...");
		break;
	}
	case LevelType::ENDLESS:
		buildBalanced(tree, def.nodes);
		tree.balancing = false;
		copyText(message, 128, "Operations are streaming in. Rotate to keep the vault standing.");
		break;
	default:
		break;
	}

	List* all = tree.inOrder_R();
	nodeCount = 0;
	for (List* c = all; c; c = c->next) nodeCount++;
	freeList(all);
	(void)log;

	if (!sim && def.type != LevelType::ENDLESS) {
		optimal = simulate(def);
		capacity = (3 * optimal + 1) / 2 + 2;       // ceil(1.5 * optimal) + 2
		settle();
	}
}

int Act3Level::simulate(const LevelDef& d) {
	Act3Level s;
	s.beginImpl(d, nullptr, true);
	int guard = 0;
	while (s.state == PLAYING && guard++ < 10000)
		if (!s.autoStep(nullptr)) break;
	return s.alarm;
}

// 3.1 badges 

bool Act3Level::clickNode(int id, EventLog* log) {
	if (state != PLAYING) return false;
	if (def.type == LevelType::ORDER) {
		if (phase != PH_PLAY) return false;
		AVLNode* cur = findById(tree.getTree(), curId);
		if (!cur) return false;
		if (cur->left && cur->left->id == id) return move(1, log);
		if (cur->right && cur->right->id == id) return move(2, log);
		copyText(message, 128, "You can only move to a room next to you.");
		return false;
	}
	if (def.type != LevelType::BADGES || id <= 0 || id >= 256) return false;
	AVLNode* n = findById(tree.getTree(), id);
	if (!n) return false;
	if (clicked[id]) { copyText(message, 128, "Already marked."); return false; }
	int bf = tree.balanceFactor(n);
	if (bf > 1 || bf < -1) {
		clicked[id] = true; found++; alarm++;
		if (log) { log->add(EventTypes::HIGHLIGHT_SUBTREE, id, 2); log->add(EventTypes::DONE); }
		copyText(message, 128, "Unbalanced room found.");
		if (found >= unbalancedTotal) { state = WON; copyText(message, 128, "Every unbalanced room found!"); }
		settle();
		return true;
	}
	copyText(message, 128, "That room is balanced (+1 alarm): its balance is ");
	appendInt(message, 128, bf);
	appendText(message, 128, ".");
	penalize(1, 0);
	return false;
}

// 3.2 / 3.3 / endless rotations 

bool Act3Level::answerCase(int c, EventLog* log) {
	(void)log;
	if (state != PLAYING || def.type != LevelType::DOUBLE || identified) return false;
	if (c == caseId) {
		identified = true;
		copyText(message, 128, "Correct: ");
		appendText(message, 128, caseName(caseId));
		appendText(message, 128, ". Now perform the two rotations.");
		return true;
	}
	copyText(message, 128, "Not quite (+2 alarm, -50): it is ");
	appendText(message, 128, caseName(caseId));
	appendText(message, 128, ".");
	penalize(2, 50);
	return false;
}

void Act3Level::checkInvalid() {
	if (def.type != LevelType::ENDLESS || state != PLAYING) return;
	int ids[256];
	int cnt = unbalancedIds(ids, 256);
	bool deep = false;
	for (int i = 0; i < cnt; i++) {
		AVLNode* n = findById(tree.getTree(), ids[i]);
		int bf = tree.balanceFactor(n);
		if (bf >= MAX_BALANCE || bf <= -MAX_BALANCE) deep = true;
	}
	if (deep) {                 // balance >= 3 means a second hit landed before the first was repaired
		state = LOST;
		copyText(message, 128, "The vault collapsed! You survived ");
		appendInt(message, 128, (int)survived);
		appendText(message, 128, " seconds.");
	}
}

bool Act3Level::rotate(int id, bool left, EventLog* log) {
	if (state != PLAYING) return false;
	if (def.type == LevelType::ENDLESS) {
		bool ok = tree.rotateAt(id, left, log);
		if (ok && log) log->add(EventTypes::DONE);
		if (!ok) copyText(message, 128, "That room cannot rotate that way.");
		return ok;          // collapse is only judged when operations arrive
	}
	if (def.type != LevelType::SINGLE && def.type != LevelType::DOUBLE) return false;
	if (!identified) { copyText(message, 128, "Identify the case first (LL, RR, LR or RL)."); return false; }
	if (moveStep >= moveCount) return false;
	if (id != mvId[moveStep] || left != mvLeft[moveStep]) {
		copyText(message, 128, "Wrong rotation (+2 alarm, -50 points) - it is a ");
		appendText(message, 128, caseName(caseId));
		appendText(message, 128, " case.");
		penalize(2, 50);
		return false;
	}
	if (!tree.rotateAt(id, left, log)) return false;
	if (log) log->add(EventTypes::DONE);
	alarm++;
	moveStep++;
	if (moveStep >= moveCount) {
		round++;
		if (round >= def.rounds) { state = WON; copyText(message, 128, "All repairs done!"); }
		else { setupViolationRound(); }
	}
	else copyText(message, 128, "Good. Now the second rotation.");
	settle();
	return true;
}

// 3.4 predict 

bool Act3Level::predict(int c, EventLog* log) {
	if (state != PLAYING || def.type != LevelType::PREDICT) return false;
	if (c == expected) copyText(message, 128, "Correct prediction!");
	else {
		copyText(message, 128, "Not quite (+2 alarm, -50): inserting ");
		appendInt(message, 128, pendingKey);
		appendText(message, 128, " causes ");
		appendText(message, 128, caseName(expected));
		appendText(message, 128, ".");
		penalties++; penaltyPoints += 50; alarm += 2;
	}
	tree.insert(pendingKey, 0, log);          // the vault repairs itself
	lastInsertKey = pendingKey;
	alarm++;
	round++;
	if (round >= def.rounds) state = WON; else pickPredictKey();
	settle();
	return true;
}

// 3.5 order statistics 

void Act3Level::enterOrder(TreeNode* n, EventLog* log) {
	curId = n->id;
	alarm++;
	if (log) log->add(EventTypes::VISIT, n->id);
	AVLNode* a = static_cast<AVLNode*>(n);
	int L = tree.getSize(static_cast<AVLNode*>(a->left));
	if (kRemain == L + 1) {
		state = WON;
		if (log) log->add(EventTypes::HIGHLIGHT_SUBTREE, n->id, 2);
		copyText(message, 128, "Found the treasure using only subtree sizes!");
	}
	if (log) log->add(EventTypes::DONE);
	settle();
}

void Act3Level::deliverNext(EventLog* log) {
	if (def.type != LevelType::ORDER || phase != PH_DELIVERY) return;
	int n = def.nodes < 3 ? 3 : (def.nodes > 60 ? 60 : def.nodes);
	int key = deliverKeys[delivered++];
	EventLog tmp;
	tree.insert(key, 0, &tmp);
	lastInsertKey = key;
	for (int i = 0; i < tmp.size(); i++) {
		const Events& e = tmp.get(i);
		if (e.type == EventTypes::ROTATE) deliveryRotations++;
		if (!log || e.type == EventTypes::DONE || e.type == EventTypes::COMPARE) continue;
		log->add(e.type, e.id, e.extra, e.data);
	}
	if (log) log->add(EventTypes::DONE);
	if (delivered >= n) {
		phase = PH_PLAY;
		kWanted = def.target > 0 ? def.target : 1 + rng.upto(n);
		if (kWanted > n) kWanted = n;
		needIdx = n - kWanted + 1;                 // k-th largest = (n-k+1)-th smallest
		kRemain = needIdx;
		AVLNode* t = tree.kth(needIdx);
		targetKey = t ? t->key : 0;
		copyText(message, 128, "Steal the treasure that is number ");
		appendInt(message, 128, kWanted);
		appendText(message, 128, " by value (largest = 1). Use the subtree sizes.");
		nodeCount = n;
		enterOrder(tree.getTree(), log);
	}
}

int Act3Level::orderDir() {
	AVLNode* cur = findById(tree.getTree(), curId);
	if (!cur) return 0;
	int L = tree.getSize(static_cast<AVLNode*>(cur->left));
	if (kRemain == L + 1) return 0;
	return kRemain <= L ? 1 : 2;
}

int Act3Level::correctDir() { return orderDir(); }

bool Act3Level::move(int dir, EventLog* log) {
	if (state != PLAYING || def.type != LevelType::ORDER || phase != PH_PLAY) return false;
	AVLNode* cur = findById(tree.getTree(), curId);
	if (!cur) return false;
	int want = orderDir();
	if (want == 0) return false;
	int L = tree.getSize(static_cast<AVLNode*>(cur->left));
	if (dir != want) {
		copyText(message, 128, "Wrong way (+1 alarm): need number ");
		appendInt(message, 128, kRemain);
		appendText(message, 128, " from the smallest here; the left side has ");
		appendInt(message, 128, L);
		appendText(message, 128, want == 1 ? " rooms, so go left." : " rooms, so go right.");
		penalize(1, 0);
		return false;
	}
	TreeNode* child = (dir == 1) ? cur->left : cur->right;
	if (!child) return false;
	if (dir == 2) kRemain -= (L + 1);
	enterOrder(child, log);
	return true;
}

// endless 

void Act3Level::applyOp(EventLog* log) {
	List* l = tree.inOrder_R();
	int keys[256]; int n = 0;
	for (List* c = l; c && n < 256; c = c->next) keys[n++] = c->val;
	freeList(l);
	bool doInsert = (n < 6) || rng.upto(100) < 60;
	if (doInsert) {
		int k = 0;
		for (int g = 0; g < 1000; g++) { k = 1 + rng.upto(99); if (!tree.search(k)) break; }
		tree.insert(k, 0, log);
		lastInsertKey = k;
	}
	else tree.remove(keys[rng.upto(n)], log);
	opsApplied++;
}

bool Act3Level::tick(float dt, EventLog* log) {
	if (def.type != LevelType::ENDLESS || state != PLAYING) return false;
	survived += dt;
	opTimer += dt;
	float interval = (float)(def.rate > 0 ? def.rate : 4) - survived * 0.05f;     // difficulty ramps up
	if (interval < 0.8f) interval = 0.8f;
	bool applied = false;
	if (log && opTimer >= interval) {
		opTimer = 0;
		applyOp(log);
		applied = true;
	}
	checkInvalid();
	return applied;
}

// hint / bot / results 

void Act3Level::hint() {
	if (state != PLAYING) return;
	hintUsed = true; hintId = 0;
	switch (def.type) {
	case LevelType::BADGES: {
		int ids[256]; int cnt = unbalancedIds(ids, 256);
		for (int i = 0; i < cnt; i++) if (!clicked[ids[i]]) { hintId = ids[i]; break; }
		copyText(message, 128, "Hint: the highlighted room is unbalanced.");
		break;
	}
	case LevelType::SINGLE: case LevelType::DOUBLE:
		if (!identified) { copyText(message, 128, "Hint: it is a "); appendText(message, 128, caseName(caseId)); appendText(message, 128, " case."); }
		else if (moveStep < moveCount) {
			hintId = mvId[moveStep];
			copyText(message, 128, mvLeft[moveStep] ? "Hint: rotate the highlighted room LEFT." : "Hint: rotate the highlighted room RIGHT.");
		}
		break;
	case LevelType::PREDICT:
		copyText(message, 128, "Hint: expect "); appendText(message, 128, caseName(expected)); appendText(message, 128, ".");
		break;
	case LevelType::ORDER:
		if (phase == PH_PLAY) {
			AVLNode* cur = findById(tree.getTree(), curId);
			int d = orderDir();
			if (cur && d) { TreeNode* c = (d == 1) ? cur->left : cur->right; hintId = c ? c->id : 0; }
			copyText(message, 128, d == 1 ? "Hint: go left." : "Hint: go right.");
		}
		break;
	case LevelType::ENDLESS: {
		AVLNode* z = lowestUnbalanced(tree);
		if (z) { hintId = z->id; copyText(message, 128, "Hint: repair the highlighted room ("); appendText(message, 128, caseName(classify(tree, z))); appendText(message, 128, ")."); }
		else copyText(message, 128, "Nothing is unbalanced right now.");
		break;
	}
	default: break;
	}
}

bool Act3Level::autoStep(EventLog* log) {
	if (state != PLAYING) return false;
	switch (def.type) {
	case LevelType::BADGES: {
		int ids[256]; int cnt = unbalancedIds(ids, 256);
		for (int i = 0; i < cnt; i++) if (!clicked[ids[i]]) return clickNode(ids[i], log);
		return false;
	}
	case LevelType::SINGLE: case LevelType::DOUBLE:
		if (!identified) return answerCase(caseId, log);
		return moveStep < moveCount && rotate(mvId[moveStep], mvLeft[moveStep], log);
	case LevelType::PREDICT:
		return predict(expected, log);
	case LevelType::ORDER:
		if (phase == PH_DELIVERY) { deliverNext(log); return true; }
		return move(orderDir(), log);
	case LevelType::ENDLESS: {
		if (botHasNext) { botHasNext = false; return rotate(botNextId, botNextLeft, log); }
		AVLNode* z = lowestUnbalanced(tree);
		if (!z) return false;
		int c = classify(tree, z);
		if (c == Act3Case::LL) return rotate(z->id, false, log);
		if (c == Act3Case::RR) return rotate(z->id, true, log);
		if (c == Act3Case::LR) { botHasNext = true; botNextId = z->id; botNextLeft = false; return rotate(z->left->id, true, log); }
		botHasNext = true; botNextId = z->id; botNextLeft = true;                       // RL
		return rotate(z->right->id, false, log);
	}
	}
	return false;
}

int Act3Level::stars() const {
	if (def.type == LevelType::ENDLESS) {
		int s = (int)survived;
		return s >= 120 ? 3 : (s >= 60 ? 2 : (s >= 30 ? 1 : 0));
	}
	if (state != WON) return 0;
	int s;
	if (alarm <= optimal && penalties == 0) s = 3;
	else if (alarm * 4 <= optimal * 5 && penalties <= 1) s = 2;
	else s = 1;
	if (hintUsed && s > 2) s = 2;
	return s;
}

int Act3Level::score() const {
	if (def.type == LevelType::ENDLESS) return (int)(survived * 10.0f);      // survival time
	int v = 1000 - 10 * alarm - penaltyPoints + bonus;
	return v < 0 ? 0 : v;
}