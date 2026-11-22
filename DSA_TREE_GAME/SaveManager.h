#pragma once
// Save / load 
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