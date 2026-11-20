#pragma once
#include"Events.h"

class MaxHeap {
private:
	int* arr;
	int count;   
	int total;   
	int cap;

	static int cmp(int a, int b);            
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

	int size() const { return count; }             
	int totalSize() const { return total; }        
	int sortedCount() const { return total - count; }
	int get(int i) const { return (i >= 0 && i < total) ? arr[i] : 0; }
	const int* data() const { return arr; }

	bool isMaxHeap() const;            
	bool isSortedRegionValid() const;  
	bool validate() const;             

	void display() const;

	void buildMaxHeap(int* A, int n, EventLog* log = nullptr);
	void loadArray(const int* A, int n);      
	bool swapAt(int i, int j, EventLog* log = nullptr);  
	void siftDown(int i, int heapSize, EventLog* log = nullptr);
	void siftUp(int i, EventLog* log = nullptr);
	bool insert(int value, EventLog* log = nullptr);      
	bool extractMax(int& out, EventLog* log = nullptr);   
	void heapSort(int* A, int n, EventLog* log = nullptr);
};