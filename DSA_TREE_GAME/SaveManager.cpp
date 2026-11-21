#include "SaveManager.h"
#include "Validator.h"
#include "TextUtil.h"
#include <fstream>
#include <cstring>

// helpers 

static bool isWs(char c) { return c == ' ' || c == '\n' || c == '\r' || c == '\t'; }

static bool readTok(std::istream& in, char* buf, int cap) {
	char c;
	buf[0] = 0;
	while (in.get(c) && isWs(c)) {}
	if (!in) return false;
	int n = 0;
	do {
		if (n >= cap - 1) return false;        // token too long = corrupt
		buf[n++] = c;
	} while (in.get(c) && !isWs(c));
	buf[n] = 0;
	return true;
}

static bool parseIntTok(const char* s, int& v) {
	int i = 0; bool neg = false;
	if (s[0] == '-') { neg = true; i = 1; }
	if (!s[i]) return false;
	long long x = 0; int digits = 0;
	for (; s[i]; i++) {
		if (s[i] < '0' || s[i] > '9' || ++digits > 10) return false;
		x = x * 10 + (s[i] - '0');
	}
	if (neg) x = -x;
	if (x > 2000000000LL || x < -2000000000LL) return false;
	v = (int)x;
	return true;
}

static bool readInt(std::istream& in, int& v) {
	char t[24];
	return readTok(in, t, 24) && parseIntTok(t, v);
}

static void freeTree(TreeNode* n) {
	if (!n) return;
	freeTree(n->left);
	freeTree(n->right);
	delete n;
}

static TreeNode* readTreeRec(std::istream& in, bool avl, int& nextId, int& count, int depth, bool& ok) {
	char tok[24];
	if (!readTok(in, tok, 24)) { ok = false; return nullptr; }
	if (tok[0] == '#' && tok[1] == 0) return nullptr;
	int key;
	if (!parseIntTok(tok, key) || depth > 3000 || count > 100000) { ok = false; return nullptr; }
	count++;
	TreeNode* n;
	if (avl) n = new AVLNode(nextId, key, 0); else n = new BSTNode(nextId, key, 0);
	nextId++;
	n->left = readTreeRec(in, avl, nextId, count, depth + 1, ok);
	if (!ok) { freeTree(n); return nullptr; }
	n->right = readTreeRec(in, avl, nextId, count, depth + 1, ok);
	if (!ok) { freeTree(n); return nullptr; }
	return n;
}

static void writeTree(std::ostream& out, TreeNode* root) {
	treeStack st;
	st.push(root);
	bool firstTok = true;
	while (!st.isEmpty()) {
		TreeNode* n = st.peek();
		st.pop();
		if (!firstTok) out << ' ';
		firstTok = false;
		if (n == nullptr) { out << '#'; continue; }
		out << n->val;
		st.push(n->right);
		st.push(n->left);
	}
}

// save 

int SaveManager::saveGame(const char* path, const Progress& p, int seed,
	const BST* tree, int treeKind, const MaxHeap* heap, char* err, int errCap) {
	if (err) err[0] = 0;
	std::ofstream out(path);
	if (!out) { if (err) tCopy(err, errCap, "Could not open the save file for writing."); return SaveResult::WRITE_ERROR; }

	out << "VAULT_SAVE 1\n";
	for (int a = 1; a <= 4; a++) {
		out << "STARS " << a;
		for (int i = 0; i < 8; i++) out << ' ' << p.stars[a][i];
		out << '\n';
	}
	out << "SEED " << seed << '\n';
	if (tree && (treeKind == KIND_BST || treeKind == KIND_AVL)) {
		out << "TREE " << treeKind << ' ';
		writeTree(out, tree->getTree());
		out << '\n';
	}
	else out << "TREE 0\n";
	if (heap) {
		out << "HEAP " << heap->totalSize() << ' ' << heap->size();
		for (int i = 0; i < heap->totalSize(); i++) out << ' ' << heap->get(i);
		out << '\n';
	}
	out << "END\n";
	out.flush();
	if (!out) { if (err) tCopy(err, errCap, "Write failed (disk full or read-only?)."); return SaveResult::WRITE_ERROR; }
	return SaveResult::OK;
}

// ---------------- load ----------------

int SaveManager::loadGame(const char* path, Progress& p, int& seed,
	BST* bstOut, AVL* avlOut, int& treeKind, MaxHeap* heapOut, bool& hasHeap, char* err, int errCap) {
	if (err) err[0] = 0;
	std::ifstream in(path);
	if (!in) return SaveResult::NOT_FOUND;

	Progress tp = p;
	int tseed = seed, tkind = KIND_NONE, nextId = 1, nodeCount = 0;
	TreeNode* root = nullptr;
	int* heapData = nullptr; int heapTotal = 0, heapActive = 0; bool gotHeap = false;
	bool sawTree = false, sawEnd = false;
	const char* problem = nullptr;

	char tok[32];
	if (!readTok(in, tok, 32) || strcmp(tok, "VAULT_SAVE") != 0) problem = "Not a Vault Heist save file.";
	int version = 0;
	if (!problem && (!readInt(in, version) || version != 1)) problem = "Unsupported save version.";

	while (!problem && !sawEnd) {
		if (!readTok(in, tok, 32)) { problem = "File is truncated (no END marker)."; break; }
		if (!strcmp(tok, "END")) { sawEnd = true; }
		else if (!strcmp(tok, "STARS")) {
			int a;
			if (!readInt(in, a) || a < 1 || a > 4) { problem = "Bad STARS line."; break; }
			for (int i = 0; i < 8 && !problem; i++) {
				int s;
				if (!readInt(in, s) || s < 0 || s > 3) problem = "Star count out of range.";
				else tp.stars[a][i] = s;
			}
		}
		else if (!strcmp(tok, "SEED")) {
			if (!readInt(in, tseed)) problem = "Bad SEED.";
		}
		else if (!strcmp(tok, "TREE")) {
			if (sawTree) { problem = "Two TREE sections."; break; }
			sawTree = true;
			if (!readInt(in, tkind) || tkind < 0 || tkind > 2) { problem = "Bad tree kind."; break; }
			if (tkind != KIND_NONE) {
				bool ok = true; int count = 0;
				root = readTreeRec(in, tkind == KIND_AVL, nextId, count, 0, ok);
				nodeCount = count;
				if (!ok) { problem = "Tree data is malformed or truncated."; root = nullptr; }
			}
		}
		else if (!strcmp(tok, "HEAP")) {
			if (gotHeap) { problem = "Two HEAP sections."; break; }
			if (!readInt(in, heapTotal) || !readInt(in, heapActive) || heapTotal < 0 || heapTotal > 100000
				|| heapActive < 0 || heapActive > heapTotal) {
				problem = "Bad HEAP header."; break;
			}
			heapData = new int[heapTotal > 0 ? heapTotal : 1];
			for (int i = 0; i < heapTotal; i++)
				if (!readInt(in, heapData[i])) { problem = "Heap data is malformed or truncated."; break; }
			gotHeap = true;
		}
		else problem = "Unknown section in save file.";
	}

	char why[96]; why[0] = 0;
	BST scratchBst; AVL scratchAvl;
	MaxHeap scratchHeap;

	// Validator runs before anything is handed to the game.
	if (!problem && tkind == KIND_BST && root) {
		scratchBst.adopt(root, nextId); root = nullptr;
		if (!Validator::bst(scratchBst.getTree(), why, 96)) problem = why;
	}
	else if (!problem && tkind == KIND_AVL && root) {
		scratchAvl.adopt(root, nextId); root = nullptr;
		scratchAvl.recompute();                         // heights and sizes are not stored
		if (!Validator::avl(scratchAvl, why, 96)) problem = why;
	}
	if (!problem && gotHeap) {
		scratchHeap.restore(heapData, heapTotal, heapActive);
		if (!Validator::heap(scratchHeap, why, 96)) problem = why;
	}

	if (problem) {
		if (err) tCopy(err, errCap, problem);
		freeTree(root);
		delete[] heapData;
		return SaveResult::INVALID;                      // caller's objects are untouched
	}

	// commit
	p = tp; seed = tseed; treeKind = tkind; hasHeap = gotHeap;
	(void)nodeCount;
	if (tkind == KIND_BST && bstOut) { TreeNode* r = scratchBst.releaseTree(); bstOut->adopt(r, nextId); }
	if (tkind == KIND_AVL && avlOut) { TreeNode* r = scratchAvl.releaseTree(); avlOut->adopt(r, nextId); }
	if (gotHeap && heapOut) heapOut->restore(heapData, heapTotal, heapActive);
	delete[] heapData;
	return SaveResult::OK;
}

// leaderboard 

int Leaderboard::load(const char* path, LeaderEntry* out, int maxOut) {
	std::ifstream in(path);
	if (!in) return 0;
	char tok[32];
	if (!readTok(in, tok, 32) || strcmp(tok, "VAULT_LEADERBOARD") != 0) return 0;
	int v;
	if (!readInt(in, v) || v != 1) return 0;
	int n = 0;
	while (n < maxOut) {
		LeaderEntry e;
		if (!readTok(in, e.name, 16)) break;
		if (!readInt(in, e.score)) break;
		out[n++] = e;
	}
	return n;
}

bool Leaderboard::submit(const char* path, const char* name, int score) {
	LeaderEntry list[MAX + 1];
	int n = load(path, list, MAX);

	LeaderEntry e;
	int len = 0;
	for (int i = 0; name && name[i] && len < 15; i++) e.name[len++] = (name[i] == ' ') ? '_' : name[i];   // no spaces in the file
	if (len == 0) { tCopy(e.name, 16, "ANON"); len = 4; }
	e.name[len] = 0;
	e.score = score;

	int pos = n;
	while (pos > 0 && list[pos - 1].score < score) { list[pos] = list[pos - 1]; pos--; }
	list[pos] = e;
	if (n < MAX) n++;

	std::ofstream out(path);
	if (!out) return false;
	out << "VAULT_LEADERBOARD 1\n";
	for (int i = 0; i < n; i++) out << list[i].name << ' ' << list[i].score << '\n';
	out.flush();
	return (bool)out;
}