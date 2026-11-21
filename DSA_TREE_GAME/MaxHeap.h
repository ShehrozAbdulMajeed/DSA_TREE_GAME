#pragma once
#include"Events.h"

class MaxHeap {
private:
	int* arr;
	int count;   
	int total;   // active heap + sorted region
	int cap;

	static int cmp(int a, int b);            // -1, 0, 1
	void ensureCap(int n);
	void swapValues(int i, int j);
	void siftDownImpl(int i, int heapSize, EventLog* log);   
	void siftUpImpl(int i, EventLog* log);                   
	void buildInternal(const int* A, int n, EventLog* log);  

public:
	MaxHeap();
	~MaxHeap();
	MaxHeap(const MaxHeap&) = delete;
	MaxHeap& operator=(const MaxHeap&) = delete;

	// accessors for the Array View / tree view 
	int size() const { return count; }             // active heap size
	int totalSize() const { return total; }        // active + sorted
	int sortedCount() const { return total - count; }
	int get(int i) const { return (i >= 0 && i < total) ? arr[i] : 0; }
	const int* data() const { return arr; }

	// validators
	bool isMaxHeap() const;            // every parent >= children, active region only
	bool isSortedRegionValid() const;  // ascending, and every value >= every active value
	bool validate() const;             // both of the above

	void display() const;

	// operations 
	void buildMaxHeap(int* A, int n, EventLog* log = nullptr);
	bool restore(const int* A, int totalSize, int activeSize);   // load a saved heap + sorted region
	void loadArray(const int* A, int n);      // copy without heapifying
	bool swapAt(int i, int j, EventLog* log = nullptr);   // player swap
	void siftDown(int i, int heapSize, EventLog* log = nullptr);
	void siftUp(int i, EventLog* log = nullptr);
	bool insert(int value, EventLog* log = nullptr);      // false if a sorted region exists
	bool extractMax(int& out, EventLog* log = nullptr);   // false if the heap is empty
	void heapSort(int* A, int n, EventLog* log = nullptr);
};