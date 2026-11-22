#pragma once
// Each function returns true if the structure is valid
#include "AVL.h"
#include "MaxHeap.h"

struct Validator {
	// BST: keys strictly increasing in order (so every left key < node < every right key),
	// val == key on every node, no cycles, no shared nodes, unique ids.
	static bool bst(TreeNode* root, char* why, int cap);
	// AVL: everything above, plus stored height/size equal recomputed values and |balance| <= 1.
	static bool avl(AVL& tree, char* why, int cap);
	// Max heap: every parent >= its children in the active region, sorted region ascending and >= heap.
	static bool heap(const MaxHeap& h, char* why, int cap);
};