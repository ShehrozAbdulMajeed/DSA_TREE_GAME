#include"Events.h"
using namespace std;
#pragma once

class MaxHeap {
private:
int* arr;
int count;
int cap;
public:
MaxHeap();
void buildMaxHeap(int* A, int n, EventLog* log = nullptr);
bool isMaxHeap();
void display();
int extractMax(EventLog* log = nullptr);
void siftDown(int i, int heapSize, EventLog* log = nullptr);
void siftUp(int i, EventLog* log = nullptr);
void insert(int value, EventLog* log = nullptr);
void heapSort(int* A, int n, EventLog* log = nullptr);
};