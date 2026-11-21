#include <iostream>
#include "game/act4/HeapRepairLevel.h"

using namespace std;
using namespace vh;

static DynArray<int> disturbedDownHeap() {
    DynArray<int> values;
    values.push(20);
    values.push(90);
    values.push(84);
    values.push(31);
    values.push(72);
    values.push(79);
    values.push(77);
    return values;
}

int runHeapRepairLevelTests() {
    int failed = 0;
    auto check = [&](bool ok, const char* name) {
        cout << (ok ? "PASS " : "FAIL ") << name << endl;
        if (!ok) failed++;
    };

    HeapRepairLevel level;
    level.load(disturbedDownHeap());
    check(level.isValidSwap(0, 1),
          "repair level accepts parent-child swap on a broken edge");

    bool accepted = true;
    EventLog wrong = level.chooseSwap(0, 2, accepted);
    check(!accepted && wrong.empty() && level.score().alarm() == 1,
          "invalid repair swap adds one alarm penalty");

    EventLog repair = level.chooseSwap(0, 1, accepted);
    check(accepted && !repair.empty() && !level.isComplete() &&
          level.valueAt(0) == 90 && level.valueAt(1) == 20,
          "first valid swap lifts the larger child toward the root");

    EventLog finish = level.chooseSwap(1, 4, accepted);
    check(accepted && !finish.empty() && level.isComplete() &&
          level.valueAt(0) == 90 && level.valueAt(1) == 72,
          "manual swaps restore the Max Heap and complete the level");

    HeapRepairLevel longerRoute;
    longerRoute.load(disturbedDownHeap());
    EventLog first = longerRoute.chooseSwap(0, 1, accepted);
    check(accepted && !first.empty() && !longerRoute.isComplete(),
          "valid first swap may improve the heap without completing it");
    EventLog second = longerRoute.chooseSwap(1, 4, accepted);
    check(accepted && !second.empty() && longerRoute.isComplete() &&
          longerRoute.score().steps() == 2,
          "a longer valid repair route is allowed but costs an extra step");

    HeapRepairLevel undoLevel;
    undoLevel.load(disturbedDownHeap());
    undoLevel.chooseSwap(0, 1, accepted);
    check(!undoLevel.isComplete() && undoLevel.valueAt(0) == 90, "setup for repair undo");
    check(undoLevel.undo() && !undoLevel.isComplete() && undoLevel.valueAt(0) == 20 &&
          undoLevel.score().steps() == 0,
          "repair level undo restores heap, complete flag, and score");

    return failed;
}
