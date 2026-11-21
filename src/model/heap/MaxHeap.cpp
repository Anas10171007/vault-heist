#include "model/heap/MaxHeap.h"
#include "model/heap/HeapValidator.h"
#include <cassert>
#include <sstream>
#include <string>

namespace vh {

MaxHeap::MaxHeap(bool recordEvents)
    : heapSize(0), comparisonCount(0), swapCount(0), recording(recordEvents) {}

void MaxHeap::load(const DynArray<int>& input) {
    values = input;
    heapSize = values.size();
    resetCounters();
}

int MaxHeap::parentIndex(int index) const {
    return (index - 1) / 2;
}

int MaxHeap::leftIndex(int index) const {
    return index * 2 + 1;
}

int MaxHeap::rightIndex(int index) const {
    return index * 2 + 2;
}

void MaxHeap::swapValues(int first, int second, int reason, EventLog& events) {
    int firstValue = values[first];
    int secondValue = values[second];
    values[first] = secondValue;
    values[second] = firstValue;
    swapCount++;
    events.append(Event(EventType::HEAP_SWAP, first, second, Side::NONE, 0,
                        firstValue, secondValue, reason));
}

void MaxHeap::heapifyUp(int index, EventLog& events) {
    events.append(Event(EventType::HEAPIFY_START, index, heapSize));
    int startIndex = index;

    while (index > 0) {
        int parent = parentIndex(index);
        comparisonCount++;
        if (values[parent] >= values[index]) break;
        swapValues(parent, index, HeapReason::SIFT_UP, events);
        index = parent;
    }

    events.append(Event(EventType::HEAPIFY_DONE, startIndex, heapSize));
}

void MaxHeap::heapifyDown(int index, int activeLimit, int reason, EventLog& events) {
    events.append(Event(EventType::HEAPIFY_START, index, activeLimit));
    int startIndex = index;

    while (true) {
        int largest = index;
        int left = leftIndex(index);
        int right = rightIndex(index);

        if (left < activeLimit) {
            comparisonCount++;
            if (values[left] > values[largest]) largest = left;
        }
        if (right < activeLimit) {
            comparisonCount++;
            if (values[right] > values[largest]) largest = right;
        }

        if (largest == index) break;
        swapValues(index, largest, reason, events);
        index = largest;
    }

    events.append(Event(EventType::HEAPIFY_DONE, startIndex, activeLimit));
}

void MaxHeap::buildMaxHeap(EventLog& events) {
    heapSize = values.size();
    int lastParent = heapSize / 2 - 1;
    events.append(Event(EventType::HEAP_BUILD, lastParent, heapSize));

    for (int index = lastParent; index >= 0; index--) {
        heapifyDown(index, heapSize, HeapReason::BUILD, events);
    }
}

EventLog MaxHeap::buildMaxHeap() {
    EventLog events(recording);
    buildMaxHeap(events);
    events.append(Event(EventType::DONE));
    debugValidate();
    return events;
}

bool MaxHeap::canInsert() const {
    return heapSize == values.size();
}

EventLog MaxHeap::insert(int value) {
    EventLog events(recording);
    if (!canInsert()) return events;

    int oldSize = heapSize;
    values.push(value);
    heapSize++;
    events.append(Event(EventType::INSERT, oldSize, 0, Side::NONE, value));
    events.append(Event(EventType::HEAP_SIZE_CHANGE, 0, 0, Side::NONE, 0,
                        oldSize, heapSize));
    heapifyUp(heapSize - 1, events);
    events.append(Event(EventType::DONE));
    debugValidate();
    return events;
}

bool MaxHeap::extractMax(int& maximum, EventLog& events) {
    if (heapSize <= 0) return false;

    int oldSize = heapSize;
    maximum = values[0];
    events.append(Event(EventType::EXTRACT_MAX, 0, heapSize, Side::NONE, maximum));

    if (heapSize > 1) {
        swapValues(0, heapSize - 1, HeapReason::EXTRACT, events);
    }

    heapSize--;
    events.append(Event(EventType::HEAP_SIZE_CHANGE, 0, 0, Side::NONE, 0,
                        oldSize, heapSize));
    events.append(Event(EventType::MARK_SORTED, heapSize, 0, Side::NONE, values[heapSize]));

    if (heapSize > 0) heapifyDown(0, heapSize, HeapReason::SIFT_DOWN, events);
    return true;
}

EventLog MaxHeap::extractMax(int& maximum) {
    EventLog events(recording);
    if (extractMax(maximum, events)) events.append(Event(EventType::DONE));
    debugValidate();
    return events;
}

EventLog MaxHeap::heapSort() {
    EventLog events(recording);
    buildMaxHeap(events);

    int maximum = 0;
    while (heapSize > 0) extractMax(maximum, events);

    events.append(Event(EventType::HEAP_SORT_COMPLETE, values.size()));
    events.append(Event(EventType::DONE));
    debugValidate();
    return events;
}

EventLog MaxHeap::playerSwap(int first, int second) {
    EventLog events(recording);
    if (first < 0 || first >= heapSize || second < 0 || second >= heapSize ||
        first == second) {
        return events;
    }

    swapValues(first, second, HeapReason::PLAYER, events);
    events.append(Event(EventType::DONE));
    return events;
}

EventLog MaxHeap::repairUp(int index) {
    EventLog events(recording);
    if (index < 0 || index >= heapSize) return events;
    heapifyUp(index, events);
    events.append(Event(EventType::DONE));
    return events;
}

EventLog MaxHeap::repairDown(int index) {
    EventLog events(recording);
    if (index < 0 || index >= heapSize) return events;
    heapifyDown(index, heapSize, HeapReason::SIFT_DOWN, events);
    events.append(Event(EventType::DONE));
    return events;
}

bool MaxHeap::isMaxHeap() const {
    for (int index = 0; index < heapSize / 2; index++) {
        int left = leftIndex(index);
        int right = rightIndex(index);
        if (left < heapSize && values[index] < values[left]) return false;
        if (right < heapSize && values[index] < values[right]) return false;
    }
    return true;
}

bool MaxHeap::isSortedAscending() const {
    for (int index = 1; index < values.size(); index++) {
        if (values[index - 1] > values[index]) return false;
    }
    return true;
}

int MaxHeap::size() const {
    return values.size();
}

int MaxHeap::activeSize() const {
    return heapSize;
}

int MaxHeap::at(int index) const {
    return values[index];
}

int MaxHeap::comparisons() const {
    return comparisonCount;
}

int MaxHeap::swaps() const {
    return swapCount;
}

void MaxHeap::resetCounters() {
    comparisonCount = 0;
    swapCount = 0;
}

void MaxHeap::setRecordEvents(bool enabled) {
    recording = enabled;
}

string MaxHeap::serialize() const {
    ostringstream output;
    output << heapSize << '|';
    for (int index = 0; index < values.size(); index++) {
        if (index > 0) output << ',';
        output << values[index];
    }
    return output.str();
}

bool MaxHeap::deserialize(const string& snapshot) {
    size_t divider = snapshot.find('|');
    if (divider == string::npos) return false;

    int restoredSize = 0;
    try {
        size_t used = 0;
        restoredSize = stoi(snapshot.substr(0, divider), &used);
        if (used != divider || restoredSize < 0) return false;
    } catch (...) {
        return false;
    }

    DynArray<int> restored;
    string valuesText = snapshot.substr(divider + 1);
    istringstream input(valuesText);
    string token;
    while (getline(input, token, ',')) {
        if (token.empty()) return false;
        try {
            size_t used = 0;
            int value = stoi(token, &used);
            if (used != token.size()) return false;
            restored.push(value);
        } catch (...) {
            return false;
        }
    }
    if (restoredSize > restored.size() || (valuesText.empty() && restoredSize != 0)) return false;

    values = restored;
    heapSize = restoredSize;
    resetCounters();
    debugValidate();
    return true;
}

void MaxHeap::debugValidate() const {
#ifndef NDEBUG
    string error;
    bool valid = HeapValidator::validate(*this, error);
    (void)error;
    assert(valid);
#endif
}

}
