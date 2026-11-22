#pragma once

#include "Events.h"
using namespace std;
class MaxHeap {
private:
    int* arr;
    int count;
    int cap;
    int sortedStart;

    void resize(int newCapacity);

public:
    MaxHeap();
    ~MaxHeap();

    // Prevent accidental shallow copies of the dynamic array.
    MaxHeap(const MaxHeap&) = delete;
    MaxHeap& operator=(const MaxHeap&) = delete;

    void buildMaxHeap(int* A, int n, EventLog* log = nullptr);
    bool isMaxHeap() const;
    void display();

    int extractMax(EventLog* log = nullptr);
    void siftDown(int i, int heapSize, EventLog* log = nullptr);
    void siftUp(int i, EventLog* log = nullptr);
    void insert(int value, EventLog* log = nullptr);
    void heapSort(int* A, int n, EventLog* log = nullptr);

    // Read-only access for the SFML visualizer.
    int getCount() const;
    int getCapacity() const;
    int getValue(int index) const;

    // Gameplay support.
    void setRawArray(const int* A, int n);
    bool swapElements(int first, int second,
        EventLog* log = nullptr);
    void setValue(int index, int value);

    int totalSize() const;
    int size() const;
    int get(int index) const;

    void restore(const int* data, int total, int active);

    bool isSortedRegionValid() const;
};