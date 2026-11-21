#include <iostream>
#include "game/act4/HeapOperationLevel.h"

using namespace std;
using namespace vh;

// Keep swapping the first broken parent with its largest child until restored.
static void restoreWithSwaps(HeapOperationLevel& level) {
    while (level.needsRestore()) {
        DynArray<int> values;
        level.copyValues(values);
        int n = level.activeSize();
        bool moved = false;

        for (int parent = 0; parent < n / 2 && !moved; parent++) {
            int left = parent * 2 + 1;
            int right = parent * 2 + 2;
            if (left >= n) continue;

            int largest = left;
            if (right < n && values[right] > values[left]) largest = right;

            if (values[largest] > values[parent]) {
                bool accepted = false;
                level.chooseSwap(parent, largest, accepted);
                moved = accepted;
            }
        }

        if (!moved) break;
    }
}

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
    check(extract.activeSize() == 3 && extract.needsRestore() && !extract.isComplete(),
          "extract leaves the heap needing restore swaps");
    check(extract.score().steps() == 1, "extract counts as one player step");
    restoreWithSwaps(extract);
    check(!extract.needsRestore() && extract.activeSize() == 3 && !extract.isComplete(),
          "manual swaps restore the Max Heap after extract");
    check(extract.undo(), "undo restores the last swap snapshot");

    extract.load(values, "heap_extract");
    for (int index = 0; index < 4; index++) {
        extract.extractMaximum();
        if (extract.needsRestore()) restoreWithSwaps(extract);
    }
    check(extract.isComplete() && extract.score().stars() == 3,
          "extract level completes after every maximum is removed and restored");

    HeapOperationLevel race;
    race.load(values, "heap_race", 2);
    race.extractMaximum();
    if (race.needsRestore()) restoreWithSwaps(race);
    race.extractMaximum();
    if (race.needsRestore()) restoreWithSwaps(race);
    check(race.isComplete() && race.activeSize() == 2,
          "race level completes after the configured extraction count");

    HeapOperationLevel sort;
    sort.load(values, "heap_sort");
    for (int index = 0; index < 4; index++) {
        sort.extractMaximum();
        if (sort.needsRestore()) restoreWithSwaps(sort);
    }
    check(sort.isComplete() && sort.score().stars() == 3,
          "Heap Sort finale completes by extract and manual restore swaps");

    HeapOperationLevel invalid;
    invalid.load(values, "heap_extract");
    bool accepted = false;
    invalid.chooseSwap(0, 1, accepted);
    check(!accepted && invalid.score().penalties() == 1 && !invalid.isComplete(),
          "swap before extract is an invalid heap action");
    return failed;
}
