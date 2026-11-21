#include <iostream>
#include "game/act4/HeapOperationLevel.h"

using namespace std;
using namespace vh;

int runHeapOperationLevelTests() {
    int failed = 0;
    auto check = [&](bool ok, const char* name) {
        cout << (ok ? "PASS " : "FAIL ") << name << endl;
        if (!ok) failed++;
    };

    DynArray<int> values;
    values.push(3); values.push(9); values.push(2); values.push(7);

    HeapOperationLevel extract;
    extract.load(values, "heap_extract");
    extract.extractMaximum();
    check(extract.activeSize() == 3 && extract.needsHeapify() && !extract.isComplete(),
          "extract leaves the heap needing a restore");
    check(extract.score().steps() == 1, "extract counts as one player step");
    extract.restoreHeap();
    check(!extract.needsHeapify() && extract.activeSize() == 3 && !extract.isComplete(),
          "heapify restores the Max Heap after extract");
    check(extract.undo() && extract.needsHeapify() && extract.score().steps() == 1,
          "undo restores heap, score, and awaiting-heapify state");
    check(extract.undo() && extract.activeSize() == 4 && extract.score().steps() == 0,
          "second undo restores the pre-extract snapshot");

    extract.load(values, "heap_extract");
    for (int index = 0; index < 4; index++) {
        extract.extractMaximum();
        if (extract.needsHeapify()) extract.restoreHeap();
    }
    check(extract.isComplete() && extract.score().stars() == 3,
          "extract level completes after every maximum is removed and restored");

    HeapOperationLevel race;
    race.load(values, "heap_race", 2);
    race.extractMaximum();
    race.restoreHeap();
    race.extractMaximum();
    race.restoreHeap();
    check(race.isComplete() && race.activeSize() == 2,
          "race level completes after the configured extraction count");

    HeapOperationLevel sort;
    sort.load(values, "heap_sort");
    for (int index = 0; index < 4; index++) {
        sort.extractMaximum();
        if (sort.needsHeapify()) sort.restoreHeap();
    }
    check(sort.isComplete() && sort.score().stars() == 3,
          "Heap Sort finale completes by manual extract and heapify steps");

    HeapOperationLevel invalid;
    invalid.load(values, "heap_extract");
    invalid.restoreHeap();
    check(invalid.score().penalties() == 1 && !invalid.isComplete(),
          "heapify before extract is an invalid heap action");
    return failed;
}
