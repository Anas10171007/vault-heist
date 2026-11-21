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
    check(level.needsDown(0) && level.needsUp(1),
          "repair level recognizes both valid directions on a broken edge");

    bool accepted = true;
    EventLog wrong = level.chooseRepair(0, HeapDirection::UP, accepted);
    check(!accepted && wrong.empty() && level.score().alarm() == 1,
          "wrong repair direction adds one alarm penalty");

    EventLog repair = level.chooseRepair(0, HeapDirection::DOWN, accepted);
    check(accepted && repair.size() > 2 && level.isComplete() &&
          level.valueAt(0) == 90 && level.valueAt(1) == 72,
          "heapify down repairs the disturbed root and completes the level");

    HeapRepairLevel longerRoute;
    longerRoute.load(disturbedDownHeap());
    EventLog first = longerRoute.chooseRepair(1, HeapDirection::UP, accepted);
    check(accepted && !first.empty() && !longerRoute.isComplete(),
          "valid heapify up may improve the heap without completing it");
    EventLog second = longerRoute.chooseRepair(4, HeapDirection::UP, accepted);
    check(accepted && !second.empty() && longerRoute.isComplete() &&
          longerRoute.score().steps() == 2,
          "a longer valid repair route is allowed but costs an extra step");

    return failed;
}
