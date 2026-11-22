#include "Validator.h"
#include "StringUtil.h"
#include <cstring>


static bool fail(char* why, int cap, const char* msg) { copyText(why, cap, msg); return false; }

static bool checkTree(TreeNode* root, bool avlNodes, char* why, int cap) {
	why[0] = 0;
	const int LIMIT = 200000;
	const int SEEN = 3000;
	TreeNode** seen = new TreeNode * [SEEN];
	int seenN = 0, pushes = 0, count = 0;
	long long prev = 0; bool first = true;
	bool ok = true;
	treeStack st;
	TreeNode* cur = root;

	while (ok && (cur != nullptr || !st.isEmpty())) {
		while (cur != nullptr) {
			if (++pushes > LIMIT) { ok = fail(why, cap, "cycle detected (too many nodes)"); break; }
			st.push(cur);
			cur = cur->left;
		}
		if (!ok) break;
		cur = st.peek();
		st.pop();
		if (++count > LIMIT) { ok = fail(why, cap, "cycle detected (too many nodes)"); break; }

		BSTNode* b = dynamic_cast<BSTNode*>(cur);
		if (b == nullptr) { ok = fail(why, cap, "node is not a BST node"); break; }
		if (avlNodes && dynamic_cast<AVLNode*>(cur) == nullptr) { ok = fail(why, cap, "node is not an AVL node"); break; }
		if (b->val != b->key) { ok = fail(why, cap, "val and key differ"); break; }
		if (!first && b->val <= prev) { ok = fail(why, cap, "keys not strictly increasing (BST order or duplicate)"); break; }
		prev = b->val; first = false;

		if (seenN < SEEN) {
			for (int j = 0; j < seenN; j++)
				if (seen[j] == cur || seen[j]->id == cur->id) { ok = fail(why, cap, "shared node or duplicate id"); break; }
			if (!ok) break;
			seen[seenN++] = cur;
		}
		cur = cur->right;
	}
	delete[] seen;
	return ok;
}

bool Validator::bst(TreeNode* root, char* why, int cap) {
	return checkTree(root, false, why, cap);
}

bool Validator::avl(AVL& tree, char* why, int cap) {
	if (!checkTree(tree.getTree(), true, why, cap)) return false;
	if (!tree.isBalanced()) return fail(why, cap, "AVL height/size wrong or |balance| > 1");
	return true;
}

bool Validator::heap(const MaxHeap& h, char* why, int cap) {
	why[0] = 0;
	if (h.size() < 0 || h.size() > h.totalSize()) return fail(why, cap, "heap size out of range");
	if (!h.isMaxHeap()) return fail(why, cap, "max-heap property violated");
	if (!h.isSortedRegionValid()) return fail(why, cap, "sorted region invalid");
	return true;
}