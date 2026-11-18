#include <iostream>
#include "model/heap/MaxHeap.h"

using namespace std;
using namespace vh;

static DynArray<int> sampleValues() {
    DynArray<int> values;
    values.push(3);
    values.push(9);
    values.push(2);
    values.push(7);
    values.push(5);
    values.push(8);
    return values;
}

int runMaxHeapTests() {
    int failed = 0;
    auto check = [&](bool ok, const char* name) {
        cout << (ok ? "PASS " : "FAIL ") << name << endl;
        if (!ok) failed++;
    };

    DynArray<int> emptyValues;
    MaxHeap empty;
    empty.load(emptyValues);
    EventLog emptyBuild = empty.buildMaxHeap();
    check(empty.size() == 0 && empty.activeSize() == 0 && empty.isMaxHeap(),
          "empty heap builds safely");
    check(emptyBuild.size() == 2 && emptyBuild.at(0).type == EventType::HEAP_BUILD &&
          emptyBuild.at(1).type == EventType::DONE, "empty build still describes its operation");

    DynArray<int> oneValue;
    oneValue.push(42);
    MaxHeap one;
    one.load(oneValue);
    one.buildMaxHeap();
    check(one.size() == 1 && one.at(0) == 42 && one.isMaxHeap(), "single value is already a heap");

    MaxHeap built;
    built.load(sampleValues());
    EventLog buildEvents = built.buildMaxHeap();
    check(built.isMaxHeap() && built.at(0) == 9 && built.activeSize() == 6,
          "buildMaxHeap repairs an ordinary array");
    check(buildEvents.at(0).type == EventType::HEAP_BUILD && buildEvents.at(0).idA == 2 &&
          buildEvents.at(0).idB == 6, "HEAP_BUILD records last parent and heap size");
    bool buildReasonRecorded = false;
    for (int i = 0; i < buildEvents.size(); i++) {
        if (buildEvents.at(i).type == EventType::HEAP_SWAP &&
            buildEvents.at(i).extra == HeapReason::BUILD) {
            buildReasonRecorded = true;
        }
    }
    check(buildReasonRecorded, "build swaps record the BUILD reason");
    check(built.comparisons() > 0 && built.swaps() > 0, "build counts comparisons and swaps");

    DynArray<int> validValues;
    validValues.push(9);
    validValues.push(7);
    validValues.push(8);
    validValues.push(3);
    validValues.push(5);
    validValues.push(2);
    MaxHeap alreadyValid;
    alreadyValid.load(validValues);
    alreadyValid.buildMaxHeap();
    check(alreadyValid.isMaxHeap() && alreadyValid.swaps() == 0,
          "building an existing heap does not swap values");

    DynArray<int> duplicateValues;
    duplicateValues.push(5);
    duplicateValues.push(5);
    duplicateValues.push(3);
    duplicateValues.push(5);
    MaxHeap duplicates;
    duplicates.load(duplicateValues);
    duplicates.buildMaxHeap();
    check(duplicates.isMaxHeap(), "duplicates are allowed in a Max Heap");

    MaxHeap inserted;
    inserted.load(validValues);
    EventLog insertEvents = inserted.insert(10);
    check(inserted.at(0) == 10 && inserted.activeSize() == 7 && inserted.isMaxHeap(),
          "insert uses heapify up");
    check(insertEvents.size() > 3 && insertEvents.at(0).type == EventType::INSERT &&
          insertEvents.at(0).idA == 6 && insertEvents.at(0).side == Side::NONE &&
          insertEvents.at(0).key == 10, "heap insertion reuses INSERT with index and value");
    check(insertEvents.at(1).type == EventType::HEAP_SIZE_CHANGE &&
          insertEvents.at(1).oldValue == 6 && insertEvents.at(1).newValue == 7,
          "insert records the old and new heap sizes");

    MaxHeap extracted;
    extracted.load(sampleValues());
    extracted.buildMaxHeap();
    int maximum = 0;
    EventLog extractEvents = extracted.extractMax(maximum);
    check(maximum == 9 && extracted.activeSize() == 5 && extracted.at(5) == 9 && extracted.isMaxHeap(),
          "extractMax moves the maximum into the sorted region");

    bool sizeEventCorrect = false;
    bool extractSwapCorrect = false;
    for (int i = 0; i < extractEvents.size(); i++) {
        const Event& event = extractEvents.at(i);
        if (event.type == EventType::HEAP_SIZE_CHANGE && event.oldValue == 6 && event.newValue == 5) {
            sizeEventCorrect = true;
        }
        if (event.type == EventType::HEAP_SWAP && event.idA == 0 && event.idB == 5 &&
            event.extra == HeapReason::EXTRACT) {
            extractSwapCorrect = true;
        }
    }
    check(sizeEventCorrect, "extraction records HEAP_SIZE_CHANGE using oldValue and newValue");
    check(extractSwapCorrect, "extraction swap records its indices and reason");

    int sizeBeforeRejectedInsert = extracted.size();
    EventLog rejectedInsert = extracted.insert(100);
    check(!extracted.canInsert() && rejectedInsert.empty() && extracted.size() == sizeBeforeRejectedInsert,
          "insert is rejected while a sorted region exists");

    MaxHeap repeated;
    repeated.load(sampleValues());
    repeated.buildMaxHeap();
    int expectedMaximums[6] = {9, 8, 7, 5, 3, 2};
    bool extractionOrder = true;
    for (int i = 0; i < 6; i++) {
        int extractedValue = 0;
        repeated.extractMax(extractedValue);
        if (extractedValue != expectedMaximums[i] || !repeated.isMaxHeap()) extractionOrder = false;
    }
    check(extractionOrder && repeated.activeSize() == 0,
          "repeated extraction returns values from largest to smallest");

    MaxHeap sorted;
    sorted.load(sampleValues());
    EventLog sortEvents = sorted.heapSort();
    int expectedSorted[6] = {2, 3, 5, 7, 8, 9};
    bool exactSortedOrder = true;
    for (int i = 0; i < 6; i++) {
        if (sorted.at(i) != expectedSorted[i]) exactSortedOrder = false;
    }
    check(exactSortedOrder && sorted.isSortedAscending() && sorted.activeSize() == 0,
          "Heap Sort produces ascending order and marks the whole array sorted");
    check(sortEvents.at(sortEvents.size() - 2).type == EventType::HEAP_SORT_COMPLETE &&
          sortEvents.at(sortEvents.size() - 2).idA == 6 &&
          sortEvents.at(sortEvents.size() - 1).type == EventType::DONE,
          "Heap Sort ends with completion and done events");

    DynArray<int> negativeValues;
    negativeValues.push(-2);
    negativeValues.push(-8);
    negativeValues.push(-1);
    negativeValues.push(-5);
    MaxHeap negatives;
    negatives.load(negativeValues);
    negatives.heapSort();
    check(negatives.at(0) == -8 && negatives.at(1) == -5 && negatives.at(2) == -2 &&
          negatives.at(3) == -1, "Heap Sort handles negative values");

    MaxHeap silent(false);
    silent.load(sampleValues());
    EventLog silentEvents = silent.heapSort();
    check(silentEvents.empty() && silent.isSortedAscending(),
          "recording off keeps heap logic working without events");

    return failed;
}
