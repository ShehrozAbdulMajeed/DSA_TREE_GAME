#pragma once
// Save / load 
//   VAULT_SAVE 1
//   STARS 2 3 2 0 0 0 0 0 0          
//   SEED 2026
//   TREE 1 50 30 # # 70 60 # # #     kind (0 none, 1 BST, 2 AVL) then preorder with '#' for null
//   HEAP 6 4 9 8 5 4 3 1             total size, active heap size, then the array (incl. sorted region)
//   END                              a file without END is treated as truncated/corrupt
#include "AVL.h"
#include "MaxHeap.h"
#include "Progress.h"

struct SaveResult { static const int OK = 0, NOT_FOUND = 1, INVALID = 2, WRITE_ERROR = 3; };

struct SaveManager {
	static const int KIND_NONE = 0, KIND_BST = 1, KIND_AVL = 2;

	// tree may be null (kind NONE); heap may be null.
	static int saveGame(const char* path, const Progress& p, int seed,
		const BST* tree, int treeKind, const MaxHeap* heap, char* err, int errCap);

	static int loadGame(const char* path, Progress& p, int& seed,
		BST* bstOut, AVL* avlOut, int& treeKind, MaxHeap* heapOut, bool& hasHeap, char* err, int errCap);
};

struct LeaderEntry { char name[16]; int score; };

struct Leaderboard {
	static const int MAX = 10;
	static int load(const char* path, LeaderEntry* out, int maxOut);               // returns count (0 if no file)
	static bool submit(const char* path, const char* name, int score);             // keeps the best 10
};