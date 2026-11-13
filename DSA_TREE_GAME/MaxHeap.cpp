#include"MaxHeap.h"

MaxHeap::MaxHeap() {
	count = 0;
	cap = 16;
	arr = new int[cap];
}

void MaxHeap::buildMaxHeap(int* A, int n, EventLog* log){
if (log) log->add(EventTypes::HEAP_BUILD, n);
while (cap < n) {
cap *= 2;
int* temp = new int[cap];
delete[] arr;
arr = temp;
}
for(int i = 0; i < n; i++) arr[i] = A[i];
count = n;
if (log) log->add(EventTypes::HEAP_SIZE_CHANGE, count);
int non_leaf = (n/2)-1;
while (non_leaf >= 0) siftDown(non_leaf--,count,log);
if (log) log->add(EventTypes::DONE, count);
}

void MaxHeap::display() {
	for(int i = 0; i < count; i++) cout << arr[i] << " ";
}

bool MaxHeap::isMaxHeap() {
	int i = 0;
	while (i < count - 1) {
		int l = (2*i)+1;
		int r = (2*i)+2;
		if(l < count && arr[l] > arr[i]) return false;
		if(r < count && arr[r] > arr[i]) return false;
		i++;
	}
	return true;
}

void MaxHeap::siftDown(int i, int heapSize, EventLog* log) {
if(log) log->add(EventTypes::HEAPIFY_START, i, heapSize);
	while (true) {
		int l = (2 * i) + 1;
		int r = (2 * i) + 2;
		int curr = i;
		if (l < heapSize) {
			if (log) log->add(EventTypes::COMPARE, curr, l, arr[l] > arr[curr]);
			if(arr[l] > arr[curr]) curr = l;
		}
		if (r < heapSize) {
			if (log) log->add(EventTypes::COMPARE, curr, r, arr[r] > arr[curr]);
			if(arr[r] > arr[curr]) curr = r;
		}
		if (curr == i) break;
		if (log) log->add(EventTypes::HEAP_SWAP, i, curr);
		int temp = arr[i];
		arr[i] = arr[curr];
		arr[curr] = temp;
		i = curr;
	}
	if (log) log->add(EventTypes::HEAPIFY_DONE, i, heapSize);
}

void MaxHeap::siftUp(int i, EventLog* log) {
	if (log) log->add(EventTypes::HEAPIFY_START, i, count);
	while (i > 0) {
		int j = (i - 1) / 2;
		if (log) log->add(EventTypes::COMPARE, i, j, arr[j] < arr[i]);
		if (arr[j] < arr[i]) {
			if (log) log->add(EventTypes::HEAP_SWAP, j, i);
			int temp = arr[j];
			arr[j] = arr[i];
			arr[i] = temp;
			i = j;
		}
		else break;
	}
	if (log) log->add(EventTypes::HEAPIFY_DONE, i, count);
}

void MaxHeap::insert(int value, EventLog* log) {
	if (count == cap) {
		cap *= 2;
		int* temp = new int[cap];
		for (int i = 0; i < count; i++) temp[i] = arr[i];
		delete[] arr;
		arr = temp;
	}
	if (log) log->add(EventTypes::INSERT, value, count);
	arr[count++] = value;
	if (log) log->add(EventTypes::HEAP_SIZE_CHANGE, count);
	siftUp(count-1,log);
}

int MaxHeap::extractMax(EventLog* log) {
	if (count > 0) {
		int t = arr[0];
		if (log) log->add(EventTypes::EXTRACT_MAX, t);
		int i = 0;
		count--;
		if (log) log->add(EventTypes::HEAP_SIZE_CHANGE, count);
		arr[0] = arr[count];
		siftDown(i,count,log);
		return t;
	}
	return -99;
}

void MaxHeap::heapSort(int* A, int n, EventLog* log) {
	buildMaxHeap(A,n,log);
	int heapSize = count;
	while (heapSize > 1) {
		if (log) log->add(EventTypes::HEAP_SWAP, 0, heapSize-1);
	    int temp = arr[0];
		arr[0] = arr[heapSize-1];
		arr[heapSize-1] = temp;
		if (log) log->add(EventTypes::MARK_SORTED, heapSize-1, arr[heapSize-1]);
		heapSize--;
		if (log) log->add(EventTypes::HEAP_SIZE_CHANGE, heapSize);
		siftDown(0,heapSize,log);
	}
	for (int i = 0; i < n; i++) A[i] = arr[i];
	if (log) log->add(EventTypes::HEAP_SORT_COMPLETE, n);
}