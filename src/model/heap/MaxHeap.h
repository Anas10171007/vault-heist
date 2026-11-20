#pragma once
#include "model/EventLog.h"
#include "util/DynArray.h"

namespace vh {

class HeapValidator;

class MaxHeap {
public:
    MaxHeap(bool recordEvents = true);

    void load(const DynArray<int>& input);
    EventLog buildMaxHeap();
    EventLog insert(int value);
    EventLog extractMax(int& maximum);
    EventLog heapSort();
    EventLog playerSwap(int first, int second);

    bool isMaxHeap() const;
    bool isSortedAscending() const;
    bool canInsert() const;
    int size() const;
    int activeSize() const;
    int at(int index) const;

    int comparisons() const;
    int swaps() const;
    void resetCounters();
    void setRecordEvents(bool enabled);

private:
    friend class HeapValidator;

    DynArray<int> values;
    int heapSize;
    int comparisonCount;
    int swapCount;
    bool recording;

    int parentIndex(int index) const;
    int leftIndex(int index) const;
    int rightIndex(int index) const;
    void buildMaxHeap(EventLog& events);
    void heapifyUp(int index, EventLog& events);
    void heapifyDown(int index, int activeLimit, int reason, EventLog& events);
    void swapValues(int first, int second, int reason, EventLog& events);
    bool extractMax(int& maximum, EventLog& events);
    void debugValidate() const;
};

}
