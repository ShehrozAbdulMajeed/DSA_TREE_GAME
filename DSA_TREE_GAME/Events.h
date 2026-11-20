#pragma once
#include <iostream>

struct EventTypes {
	static const int  VISIT = 0;
	static const int  COMPARE = 1;
	static const int  INSERT = 2;
	static const int  UNLINK = 3;
	static const int  SWAP_VALUE = 4;
	static const int  ROTATE = 5;
	static const int  UPDATE_HEIGHT = 6;
	static const int  UPDATE_SIZE = 7;
	static const int  BALANCE_ALERT = 8;
	static const int  HIGHLIGHT_SUBTREE = 9;
	static const int  HEAP_BUILD = 10;
	static const int  HEAP_SWAP = 11;
	static const int  HEAPIFY_START = 12;
	static const int  HEAPIFY_DONE = 13;
	static const int  EXTRACT_MAX = 14;
	static const int  MARK_SORTED = 15;
	static const int  HEAP_SIZE_CHANGE = 16;
	static const int  HEAP_SORT_COMPLETE = 17;
	static const int  DONE = 18;
};

// ROTATE event: type field
struct RotationType {
	static const int LL = 0;   // single right rotation
	static const int RR = 1;   // single left rotation
	static const int LR = 2;   // left rotation on child, then right rotation
	static const int RL = 3;   // right rotation on child, then left rotation
};

// HEAP_SWAP event: reason field
struct SwapReason {
	static const int SIFT_DOWN = 0;
	static const int SIFT_UP = 1;
	static const int EXTRACT = 2;   // root <-> last active element
	static const int MANUAL = 3;    // player-initiated swap
};

struct Events {
	int type;
	int id;
	int extra;
	int data;
	Events(int x = 0, int y = 0, int z = 0, int w = 0);
};

class EventLog {
private:
	Events* events;
	int count;
	int cap;
public:
	EventLog();
	~EventLog();
	EventLog(const EventLog&) = delete;             // would double-delete[] events
	EventLog& operator=(const EventLog&) = delete;
	void add(int x, int y = 0, int z = 0, int w = 0);
	int size() const;
	const Events& get(int i) const;
	void clear();
	void reverse();                                 // reverses event order in place
};