#include "MaxHeap.h"
#include <iostream>

using namespace std;

MaxHeap::MaxHeap()
{
    count = 0;
    cap = 16;
    arr = new int[cap];
}

MaxHeap::~MaxHeap()
{
    delete[] arr;
    arr = nullptr;
    count = 0;
    cap = 0;
}

void MaxHeap::resize(int newCapacity)
{
    if (newCapacity <= cap)
        return;

    int* temp = new int[newCapacity];

    for (int i = 0; i < count; i++)
        temp[i] = arr[i];

    delete[] arr;
    arr = temp;
    cap = newCapacity;
}

void MaxHeap::buildMaxHeap(int* A, int n, EventLog* log)
{
    if (n < 0 || (n > 0 && A == nullptr))
        return;

    if (log)
        log->add(EventTypes::HEAP_BUILD, n);

    if (n > cap)
    {
        int newCapacity = cap;

        while (newCapacity < n)
            newCapacity *= 2;

        resize(newCapacity);
    }

    for (int i = 0; i < n; i++)
        arr[i] = A[i];

    count = n;

    if (log)
        log->add(EventTypes::HEAP_SIZE_CHANGE, count);

    int non_leaf = (n / 2) - 1;

    while (non_leaf >= 0)
    {
        siftDown(non_leaf, count, log);
        non_leaf--;
    }

    if (log)
        log->add(EventTypes::DONE, count);
}

bool MaxHeap::isMaxHeap()
{
    for (int i = 0; i < count; i++)
    {
        int l = (2 * i) + 1;
        int r = (2 * i) + 2;

        if (l < count && arr[l] > arr[i])
            return false;

        if (r < count && arr[r] > arr[i])
            return false;
    }

    return true;
}

void MaxHeap::display()
{
    for (int i = 0; i < count; i++)
        cout << arr[i] << " ";

    cout << endl;
}

void MaxHeap::siftDown(int i, int heapSize, EventLog* log)
{
    if (heapSize < 0 || heapSize > count || i < 0 || i >= heapSize)
        return;

    if (log)
        log->add(EventTypes::HEAPIFY_START, i, heapSize);

    while (true)
    {
        int l = (2 * i) + 1;
        int r = (2 * i) + 2;
        int curr = i;

        if (l < heapSize)
        {
            if (log)
                log->add(EventTypes::COMPARE, curr, l,
                    arr[l] > arr[curr]);

            if (arr[l] > arr[curr])
                curr = l;
        }

        if (r < heapSize)
        {
            if (log)
                log->add(EventTypes::COMPARE, curr, r,
                    arr[r] > arr[curr]);

            if (arr[r] > arr[curr])
                curr = r;
        }

        if (curr == i)
            break;

        if (log)
            log->add(EventTypes::HEAP_SWAP, i, curr,
                SwapReason::SIFT_DOWN);

        int temp = arr[i];
        arr[i] = arr[curr];
        arr[curr] = temp;

        i = curr;
    }

    if (log)
        log->add(EventTypes::HEAPIFY_DONE, i, heapSize);
}

void MaxHeap::siftUp(int i, EventLog* log)
{
    if (i < 0 || i >= count)
        return;

    if (log)
        log->add(EventTypes::HEAPIFY_START, i, count);

    while (i > 0)
    {
        int j = (i - 1) / 2;

        if (log)
            log->add(EventTypes::COMPARE, i, j,
                arr[j] < arr[i]);

        if (arr[j] < arr[i])
        {
            if (log)
                log->add(EventTypes::HEAP_SWAP, j, i,
                    SwapReason::SIFT_UP);

            int temp = arr[j];
            arr[j] = arr[i];
            arr[i] = temp;

            i = j;
        }
        else
        {
            break;
        }
    }

    if (log)
        log->add(EventTypes::HEAPIFY_DONE, i, count);
}

void MaxHeap::insert(int value, EventLog* log)
{
    if (count == cap)
        resize(cap * 2);

    if (log)
        log->add(EventTypes::INSERT, value, count);

    arr[count++] = value;

    if (log)
        log->add(EventTypes::HEAP_SIZE_CHANGE, count);

    siftUp(count - 1, log);
}

int MaxHeap::extractMax(EventLog* log)
{
    if (count <= 0)
        return -99;

    int maximum = arr[0];
    int lastIndex = count - 1;

    // Record and perform the actual root/last swap.
    if (lastIndex > 0)
    {
        if (log)
            log->add(EventTypes::HEAP_SWAP, 0, lastIndex,
                SwapReason::EXTRACT);

        int temp = arr[0];
        arr[0] = arr[lastIndex];
        arr[lastIndex] = temp;
    }

    if (log)
        log->add(EventTypes::EXTRACT_MAX, maximum);

    count--;

    if (log)
        log->add(EventTypes::HEAP_SIZE_CHANGE, count);

    if (count > 0)
        siftDown(0, count, log);

    if (log)
        log->add(EventTypes::MARK_SORTED, count, maximum);

    return maximum;
}

void MaxHeap::heapSort(int* A, int n, EventLog* log)
{
    if (n < 0 || (n > 0 && A == nullptr))
        return;

    buildMaxHeap(A, n, log);

    int heapSize = count;

    while (heapSize > 1)
    {
        if (log)
            log->add(EventTypes::HEAP_SWAP, 0, heapSize - 1,
                SwapReason::EXTRACT);

        int temp = arr[0];
        arr[0] = arr[heapSize - 1];
        arr[heapSize - 1] = temp;

        if (log)
            log->add(EventTypes::MARK_SORTED,
                heapSize - 1, arr[heapSize - 1]);

        heapSize--;

        if (log)
            log->add(EventTypes::HEAP_SIZE_CHANGE, heapSize);

        siftDown(0, heapSize, log);
    }

    for (int i = 0; i < n; i++)
        A[i] = arr[i];

    if (log)
        log->add(EventTypes::HEAP_SORT_COMPLETE, n);
}

int MaxHeap::getCount() const
{
    return count;
}

int MaxHeap::getCapacity() const
{
    return cap;
}

int MaxHeap::getValue(int index) const
{
    if (index < 0 || index >= count)
        return -1;

    return arr[index];
}

void MaxHeap::setRawArray(const int* A, int n)
{
    if (n < 0 || (n > 0 && A == nullptr))
        return;

    if (n > cap)
    {
        int newCapacity = cap;

        while (newCapacity < n)
            newCapacity *= 2;

        resize(newCapacity);
    }

    for (int i = 0; i < n; i++)
        arr[i] = A[i];

    count = n;
}

bool MaxHeap::swapElements(int first, int second, EventLog* log)
{
    if (first < 0 || second < 0 ||
        first >= count || second >= count)
        return false;

    if (first == second)
        return true;

    if (log)
        log->add(EventTypes::HEAP_SWAP, first, second,
            SwapReason::MANUAL);

    int temp = arr[first];
    arr[first] = arr[second];
    arr[second] = temp;

    return true;
}

void MaxHeap::setValue(int index, int value)
{
    if (index < 0 || index >= count)
        return;

    arr[index] = value;
}