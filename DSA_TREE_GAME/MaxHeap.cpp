#include"MaxHeap.h"

MaxHeap::MaxHeap() {
	count = 0;
	total = 0;
	cap = 16;
	arr = new int[cap];
}

MaxHeap::~MaxHeap() {
	delete[] arr;
	arr = nullptr;
}

int MaxHeap::cmp(int a, int b) {
	if (a < b) return -1;
	if (a > b) return 1;
	return 0;
}

void MaxHeap::ensureCap(int n) {
	if (n <= cap) return;
	int newCap = cap;
	while (newCap < n) newCap *= 2;
	int* temp = new int[newCap];
	for (int i = 0; i < total; i++) temp[i] = arr[i];
	delete[] arr;
	arr = temp;
	cap = newCap;
}

void MaxHeap::swapValues(int i, int j) {
	int t = arr[i];
	arr[i] = arr[j];
	arr[j] = t;
}

void MaxHeap::siftDownImpl(int i, int heapSize, EventLog* log) {
	int start = i;
	if (log) log->add(EventTypes::HEAPIFY_START, start, heapSize);
	while (true) {
		int l = (2 * i) + 1;
		int r = (2 * i) + 2;
		int largest = i;
		if (l < heapSize) {
			int c = cmp(arr[largest], arr[l]);
			if (log) log->add(EventTypes::COMPARE, largest, l, c);
			if (c < 0) largest = l;
		}
		if (r < heapSize) {
			int c = cmp(arr[largest], arr[r]);
			if (log) log->add(EventTypes::COMPARE, largest, r, c);
			if (c < 0) largest = r;
		}
		if (largest == i) break;
		if (log) log->add(EventTypes::HEAP_SWAP, i, largest, SwapReason::SIFT_DOWN);
		swapValues(i, largest);
		i = largest;
	}
	if (log) log->add(EventTypes::HEAPIFY_DONE, start, heapSize, i);
}

void MaxHeap::siftUpImpl(int i, EventLog* log) {
	int start = i;
	if (log) log->add(EventTypes::HEAPIFY_START, start, count);
	while (i > 0) {
		int p = (i - 1) / 2;
		int c = cmp(arr[i], arr[p]);
		if (log) log->add(EventTypes::COMPARE, i, p, c);
		if (c > 0) {
			if (log) log->add(EventTypes::HEAP_SWAP, i, p, SwapReason::SIFT_UP);
			swapValues(i, p);
			i = p;
		}
		else break;
	}
	if (log) log->add(EventTypes::HEAPIFY_DONE, start, count, i);
}

void MaxHeap::buildInternal(const int* A, int n, EventLog* log) {
	if (n < 0) n = 0;
	if (log) log->add(EventTypes::HEAP_BUILD, n, (n / 2) - 1);   
	ensureCap(n);
	for (int i = 0; i < n; i++) arr[i] = A[i];
	int old = count;
	count = n;
	total = n;
	if (log) log->add(EventTypes::HEAP_SIZE_CHANGE, count, old);
	for (int i = (n / 2) - 1; i >= 0; i--) siftDownImpl(i, count, log);
}

void MaxHeap::buildMaxHeap(int* A, int n, EventLog* log) {
	buildInternal(A, n, log);
	if (log) log->add(EventTypes::DONE, count);
}

void MaxHeap::loadArray(const int* A, int n) {
	if (n < 0) n = 0;
	ensureCap(n);
	for (int i = 0; i < n; i++) arr[i] = A[i];
	count = n;
	total = n;
}

bool MaxHeap::restore(const int* A, int totalSize, int activeSize) {
	if (totalSize < 0 || activeSize < 0 || activeSize > totalSize) return false;
	ensureCap(totalSize);
	for (int i = 0; i < totalSize; i++) arr[i] = A[i];
	count = activeSize;
	total = totalSize;
	return true;
}

bool MaxHeap::swapAt(int i, int j, EventLog* log) {
	if (i < 0 || j < 0 || i >= total || j >= total) return false;
	if (log) log->add(EventTypes::HEAP_SWAP, i, j, SwapReason::MANUAL);
	swapValues(i, j);
	if (log) log->add(EventTypes::DONE);
	return true;
}

void MaxHeap::siftDown(int i, int heapSize, EventLog* log) {
	if (i < 0 || heapSize > total || i >= heapSize) return;
	siftDownImpl(i, heapSize, log);
	if (log) log->add(EventTypes::DONE);
}

void MaxHeap::siftUp(int i, EventLog* log) {
	if (i < 0 || i >= count) return;
	siftUpImpl(i, log);
	if (log) log->add(EventTypes::DONE);
}

bool MaxHeap::insert(int value, EventLog* log) {
	if (total > count) return false;   // would overwrite the sorted region
	ensureCap(count + 1);
	if (log) log->add(EventTypes::INSERT, value, count);   // (value, index)
	arr[count] = value;
	count++;
	total = count;
	if (log) log->add(EventTypes::HEAP_SIZE_CHANGE, count, count - 1);
	siftUpImpl(count - 1, log);
	if (log) log->add(EventTypes::DONE);
	return true;
}

// Swap root with last active element, move it into the sorted region, shrink, sift down.
bool MaxHeap::extractMax(int& out, EventLog* log) {
	if (count == 0) return false;
	int oldSize = count;
	out = arr[0];
	if (log) log->add(EventTypes::EXTRACT_MAX, out, 0, oldSize);
	count--;
	if (count > 0) {
		if (log) log->add(EventTypes::HEAP_SWAP, 0, count, SwapReason::EXTRACT);
		swapValues(0, count);
	}
	if (log) {
		log->add(EventTypes::MARK_SORTED, count, out);
		log->add(EventTypes::HEAP_SIZE_CHANGE, count, oldSize);
	}
	if (count > 0) siftDownImpl(0, count, log);
	if (log) log->add(EventTypes::DONE);
	return true;
}

void MaxHeap::heapSort(int* A, int n, EventLog* log) {
	if (n < 0) n = 0;
	buildInternal(A, n, log);   // no DONE in the middle of the batch
	while (count > 1) {
		int last = count - 1;
		if (log) {
			log->add(EventTypes::EXTRACT_MAX, arr[0], 0, count);
			log->add(EventTypes::HEAP_SWAP, 0, last, SwapReason::EXTRACT);
		}
		swapValues(0, last);
		if (log) log->add(EventTypes::MARK_SORTED, last, arr[last]);
		count--;
		if (log) log->add(EventTypes::HEAP_SIZE_CHANGE, count, count + 1);
		siftDownImpl(0, count, log);
	}
	if (count == 1) {   // last remaining element is the minimum, already in place
		if (log) log->add(EventTypes::MARK_SORTED, 0, arr[0]);
		count = 0;
		if (log) log->add(EventTypes::HEAP_SIZE_CHANGE, 0, 1);
	}
	for (int i = 0; i < n; i++) A[i] = arr[i];
	if (log) {
		log->add(EventTypes::HEAP_SORT_COMPLETE, n);
		log->add(EventTypes::DONE);
	}
}

bool MaxHeap::isMaxHeap() const {
	for (int i = 1; i < count; i++)
		if (arr[(i - 1) / 2] < arr[i]) return false;
	return true;
}

bool MaxHeap::isSortedRegionValid() const {
	if (count < 0 || count > total) return false;
	for (int i = count + 1; i < total; i++)
		if (arr[i - 1] > arr[i]) return false;
	if (count > 0 && total > count && arr[0] > arr[count]) return false;
	return true;
}

bool MaxHeap::validate() const {
	return isMaxHeap() && isSortedRegionValid();
}

void MaxHeap::display() const {
	for (int i = 0; i < total; i++) std::cout << arr[i] << " ";
}