#include <iostream>
#include "model/heap/MaxHeap.h"
#include "util/Rng.h"

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

// Hand-written ascending sort for the Heap Sort reference checks. std::sort is
// banned by the TA rules, so the test suite must not lean on it.
static void referenceSortAscending(DynArray<int>& values) {
    for (int i = 0; i < values.size(); i++) {
        int smallest = i;
        for (int j = i + 1; j < values.size(); j++) {
            if (values[j] < values[smallest]) smallest = j;
        }
        if (smallest != i) {
            int temporary = values[i];
            values[i] = values[smallest];
            values[smallest] = temporary;
        }
    }
}

static bool matchesReferenceSort(const DynArray<int>& input) {
    DynArray<int> expected = input;
    referenceSortAscending(expected);

    MaxHeap heap(false);
    heap.load(input);
    heap.heapSort();

    if (heap.size() != expected.size() || heap.activeSize() != 0 ||
        !heap.isSortedAscending()) {
        return false;
    }
    for (int index = 0; index < expected.size(); index++) {
        if (heap.at(index) != expected[index]) return false;
    }
    return true;
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

    DynArray<int> reverseValues;
    for (int value = 9; value >= 1; value--) reverseValues.push(value);
    MaxHeap reverseBuilt;
    reverseBuilt.load(reverseValues);
    reverseBuilt.buildMaxHeap();
    check(reverseBuilt.isMaxHeap() && reverseBuilt.at(0) == 9,
          "buildMaxHeap repairs a reverse-sorted array");

    DynArray<int> randomValues;
    for (int value = 0; value < 20; value++) randomValues.push(value);
    Rng rng(0xBEEFu);
    rng.shuffle(randomValues);
    MaxHeap randomBuilt;
    randomBuilt.load(randomValues);
    randomBuilt.buildMaxHeap();
    check(randomBuilt.isMaxHeap() && randomBuilt.activeSize() == 20,
          "buildMaxHeap repairs a random permutation");

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

    DynArray<int> emptySortInput;
    check(matchesReferenceSort(emptySortInput),
          "Heap Sort matches reference sort on an empty array");

    DynArray<int> singletonSortInput;
    singletonSortInput.push(42);
    check(matchesReferenceSort(singletonSortInput),
          "Heap Sort matches reference sort on a singleton");

    DynArray<int> duplicateSortInput;
    duplicateSortInput.push(4);
    duplicateSortInput.push(1);
    duplicateSortInput.push(4);
    duplicateSortInput.push(2);
    duplicateSortInput.push(4);
    check(matchesReferenceSort(duplicateSortInput),
          "Heap Sort matches reference sort with duplicates");

    DynArray<int> negativeValues;
    negativeValues.push(-2);
    negativeValues.push(-8);
    negativeValues.push(-1);
    negativeValues.push(-5);
    check(matchesReferenceSort(negativeValues),
          "Heap Sort matches reference sort with negatives");

    DynArray<int> alreadySortedInput;
    for (int value = 1; value <= 8; value++) alreadySortedInput.push(value);
    check(matchesReferenceSort(alreadySortedInput),
          "Heap Sort matches reference sort on a sorted array");

    DynArray<int> reverseSortInput;
    for (int value = 8; value >= 1; value--) reverseSortInput.push(value);
    check(matchesReferenceSort(reverseSortInput),
          "Heap Sort matches reference sort on a reverse-sorted array");

    DynArray<int> randomSortInput;
    for (int value = 0; value < 25; value++) randomSortInput.push(value * 3 - 10);
    Rng sortRng(0xCAFEu);
    sortRng.shuffle(randomSortInput);
    check(matchesReferenceSort(randomSortInput),
          "Heap Sort matches reference sort on a random array");

    MaxHeap silent(false);
    silent.load(sampleValues());
    EventLog silentEvents = silent.heapSort();
    check(silentEvents.empty() && silent.isSortedAscending(),
          "recording off keeps heap logic working without events");

    MaxHeap snapshot;
    snapshot.load(sampleValues());
    snapshot.buildMaxHeap();
    string beforeExtract = snapshot.serialize();
    int snapshotMaximum = 0;
    snapshot.extractMax(snapshotMaximum);
    check(snapshot.deserialize(beforeExtract) && snapshot.activeSize() == 6 &&
          snapshot.at(0) == 9 && snapshot.isMaxHeap(),
          "heap snapshot restores values and active boundary for undo");
    string unchanged = snapshot.serialize();
    check(!snapshot.deserialize("not-a-snapshot") && snapshot.serialize() == unchanged,
          "invalid heap snapshot is rejected without changing state");
    check(!snapshot.deserialize("2|1,x,3") && snapshot.serialize() == unchanged,
          "non-integer heap snapshot token is rejected");
    check(!snapshot.deserialize("-1|1,2") && snapshot.serialize() == unchanged,
          "negative heap size snapshot is rejected");
    check(!snapshot.deserialize("1|") && snapshot.serialize() == unchanged,
          "empty values with positive heap size are rejected");
    check(snapshot.deserialize("0|") && snapshot.activeSize() == 0 && snapshot.size() == 0,
          "empty heap snapshot restores cleanly");

    MaxHeap brokenExtract;
    brokenExtract.load(sampleValues());
    brokenExtract.buildMaxHeap();
    int pendingMaximum = 0;
    brokenExtract.extractMaxNoHeapify(pendingMaximum);
    check(pendingMaximum == 9 && brokenExtract.activeSize() == 5 && !brokenExtract.isMaxHeap(),
          "extractMaxNoHeapify moves the max without restoring the heap");
    brokenExtract.repairDown(0);
    check(brokenExtract.isMaxHeap(), "repairDown restores the heap after extractMaxNoHeapify");

    return failed;
}
