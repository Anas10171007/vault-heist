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
    check(level.canSiftDown(0) && !level.canSiftUp(0),
          "disturbed root can sift down but not up");

    bool accepted = true;
    EventLog wrong = level.chooseHeapify(0, true, accepted);
    check(!accepted && wrong.empty() && level.score().alarm() == 1,
          "wrong heapify direction adds one alarm penalty");

    EventLog repair = level.chooseHeapify(0, false, accepted);
    check(accepted && !repair.empty() && level.isComplete() && level.valueAt(0) == 90,
          "heapify down from the root restores the Max Heap");

    HeapRepairLevel upCase;
    DynArray<int> upValues;
    upValues.push(50);
    upValues.push(40);
    upValues.push(30);
    upValues.push(80);
    upCase.load(upValues);
    check(upCase.canSiftUp(3), "leaf larger than parent can sift up");
    accepted = false;
    EventLog up = upCase.chooseHeapify(3, true, accepted);
    check(accepted && !up.empty() && upCase.isComplete() && upCase.valueAt(0) == 80,
          "heapify up restores the Max Heap");

    HeapRepairLevel undoLevel;
    undoLevel.load(disturbedDownHeap());
    undoLevel.chooseHeapify(0, false, accepted);
    check(undoLevel.isComplete(), "setup for repair undo");
    check(undoLevel.undo() && !undoLevel.isComplete() && undoLevel.valueAt(0) == 20 &&
              undoLevel.score().steps() == 0,
          "repair level undo restores heap, complete flag, and score");

    return failed;
}
