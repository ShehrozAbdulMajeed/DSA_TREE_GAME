#define _CRT_SECURE_NO_WARNINGS
#include "LevelData.h"
#include <cstring>

LevelDef::LevelDef() {
	id[0] = '?'; id[1] = 0;
	act = 2; type = LevelType::SEARCH;
	title[0] = 0; brief[0] = 0;
	seed = 1; nodes = 10; skew = 0; target = -1; rounds = 3;
	for (int i = 0; i < 8; i++) cases[i] = i % 3;
	lo = -1; hi = -1; shrink = 3; keyCount = 0;
	for (int i = 0; i < 64; i++) keys[i] = 0;
}

static void skipWs(const char*& p) { while (*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t') p++; }

static bool parseStr(const char*& p, char* out, int cap) {
	skipWs(p);
	if (*p != '"') return false;
	p++;
	int n = 0;
	while (*p && *p != '"') {
		char c = *p++;
		if (c == '\\' && *p) { c = *p++; if (c == 'n') c = ' '; }
		if (n < cap - 1) out[n++] = c;
	}
	if (*p != '"') return false;
	p++;
	out[n] = 0;
	return true;
}

static bool parseInt(const char*& p, int& v) {
	skipWs(p);
	bool neg = false;
	if (*p == '-') { neg = true; p++; }
	if (*p < '0' || *p > '9') return false;
	long x = 0;
	while (*p >= '0' && *p <= '9') { x = x * 10 + (*p - '0'); p++; }
	v = (int)(neg ? -x : x);
	return true;
}

static void copyStr(char* dst, int cap, const char* src) { strncpy(dst, src, cap - 1); dst[cap - 1] = 0; }

static void assignStr(LevelDef& d, const char* k, const char* v) {
	if (!strcmp(k, "id")) copyStr(d.id, 8, v);
	else if (!strcmp(k, "title")) copyStr(d.title, 48, v);
	else if (!strcmp(k, "brief")) copyStr(d.brief, 320, v);
	else if (!strcmp(k, "type")) {
		if (!strcmp(v, "search")) d.type = LevelType::SEARCH;
		else if (!strcmp(v, "insert")) d.type = LevelType::INSERT;
		else if (!strcmp(v, "remove")) d.type = LevelType::REMOVE;
		else if (!strcmp(v, "range")) d.type = LevelType::RANGE;
		else if (!strcmp(v, "boss")) d.type = LevelType::BOSS;
	}
}

static void assignInt(LevelDef& d, const char* k, int v) {
	if (!strcmp(k, "act")) d.act = v;
	else if (!strcmp(k, "seed")) d.seed = v;
	else if (!strcmp(k, "nodes")) d.nodes = v;
	else if (!strcmp(k, "skew")) d.skew = v;
	else if (!strcmp(k, "target")) d.target = v;
	else if (!strcmp(k, "rounds")) d.rounds = v > 8 ? 8 : (v < 1 ? 1 : v);
	else if (!strcmp(k, "lo")) d.lo = v;
	else if (!strcmp(k, "hi")) d.hi = v;
	else if (!strcmp(k, "shrink")) d.shrink = v;
}

static void assignArr(LevelDef& d, const char* k, const int* a, int n) {
	if (!strcmp(k, "cases")) { for (int i = 0; i < n && i < 8; i++) d.cases[i] = a[i]; }
	else if (!strcmp(k, "keys")) { d.keyCount = n > 64 ? 64 : n; for (int i = 0; i < d.keyCount; i++) d.keys[i] = a[i]; }
}

static bool parseObject(const char*& p, LevelDef& d) {
	skipWs(p);
	if (*p != '{') return false;
	p++;
	while (true) {
		skipWs(p);
		if (*p == '}') { p++; return true; }
		char key[24];
		if (!parseStr(p, key, 24)) return false;
		skipWs(p);
		if (*p != ':') return false;
		p++;
		skipWs(p);
		if (*p == '"') {
			char val[400];
			if (!parseStr(p, val, 400)) return false;
			assignStr(d, key, val);
		}
		else if (*p == '[') {
			p++;
			int arr[64]; int n = 0;
			skipWs(p);
			while (*p && *p != ']') {
				int v;
				if (!parseInt(p, v)) return false;
				if (n < 64) arr[n++] = v;
				skipWs(p);
				if (*p == ',') p++;
				skipWs(p);
			}
			if (*p != ']') return false;
			p++;
			assignArr(d, key, arr, n);
		}
		else {
			int v;
			if (!parseInt(p, v)) return false;
			assignInt(d, key, v);
		}
		skipWs(p);
		if (*p == ',') p++;
	}
}

int parseLevels(const char* text, LevelDef* out, int maxOut) {
	const char* p = strstr(text, "\"levels\"");
	if (!p) return 0;
	p = strchr(p, '[');
	if (!p) return 0;
	p++;
	int n = 0;
	while (n < maxOut) {
		skipWs(p);
		if (*p == ']' || *p == 0) break;
		LevelDef d;
		if (!parseObject(p, d)) break;
		out[n++] = d;
		skipWs(p);
		if (*p == ',') p++;
	}
	return n;
}

const char* kDefaultLevelsJson = R"JSON({
"levels": [
 {"id":"2.1","act":2,"type":"search","title":"Search Heist","seed":101,"nodes":10,
  "brief":"Slip into the vault and reach the target room. At every room compare the target key with the room's key: smaller means go left, larger means go right. Every room you enter raises the alarm, so do not wander."},
 {"id":"2.2","act":2,"type":"insert","title":"Stash Loot","seed":202,"nodes":9,
  "brief":"You are carrying a new treasure. Walk down the tree choosing left or right until you reach the empty spot where it belongs, and the new room will appear there. Duplicates are never allowed in a BST."},
 {"id":"2.3","act":2,"type":"remove","title":"Steal and Collapse","seed":303,"nodes":12,"rounds":3,"cases":[0,1,2],
  "brief":"Steal treasures and the rooms collapse. For each target, decide which case applies: a leaf, one child, or two children. With two children the inorder successor (smallest key on the right) takes its place."},
 {"id":"2.4","act":2,"type":"range","title":"Range Raid","seed":404,"nodes":14,
  "brief":"Collect every treasure whose key lies in the range [lo, hi]. Click rooms next to ones you already entered. Rooms that cannot hold keys in the range turn grey: a smart thief never enters them."},
 {"id":"2.5","act":2,"type":"boss","title":"Boss: The Lazy Architect","seed":505,"nodes":12,"shrink":3,
  "brief":"The architect receives deliveries in sorted order, so the vault becomes a corridor. Find the last treasure before the alarm capacity shrinks away. Afterwards you will see why balancing matters."}
]
})JSON";