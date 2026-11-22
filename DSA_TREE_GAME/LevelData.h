#pragma once
struct LevelType {
	// Act 2
	static const int SEARCH = 0, INSERT = 1, REMOVE = 2, RANGE = 3, BOSS = 4;
	// Act 3
	static const int BADGES = 5, SINGLE = 6, DOUBLE = 7, PREDICT = 8, ORDER = 9, ENDLESS = 10;
};

struct Rng {
	unsigned int s;
	Rng(unsigned int seed = 1) : s(seed) {}
	int next() { s = s * 1103515245u + 12345u; return (int)((s >> 16) & 0x7fff); }
	int upto(int n) { return n > 0 ? next() % n : 0; }
};

struct LevelDef {
	char id[8];          // "2.1"
	int act;             // 2
	int type;            // LevelType
	char title[48];
	char brief[320];
	int seed;
	int nodes;           // tree size
	int skew;            // 0..100: % chance the next generated key is bigger than all so far
	int target;          // search/insert key, -1 = chosen from the seed
	int rounds;          // remove: number of deletions
	int cases[8];        // remove: wanted case per round (0 leaf, 1 one child, 2 two children)
	int lo, hi;          // range, -1 = chosen from the seed
	int shrink;          // boss: seconds between alarm-capacity drops
	int rate;            // endless: starting seconds between operations
	int keys[64];        // optional hand-made tree: insertion order (overrides generation)
	int keyCount;
	LevelDef();
};

int parseLevels(const char* text, LevelDef* out, int maxOut);

extern const char* kDefaultLevelsJson;