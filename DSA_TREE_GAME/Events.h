#pragma once
#include<iostream>
using namespace std;

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
	void add(int x, int y = 0, int z = 0, int w = 0);
	int size() const;
	const Events& get(int i) const;
	void clear();
};