#include <iostream>
#include "game/act4/HeapBuildLevel.h"

using namespace std;
using namespace vh;

int runHeapBuildLevelTests() {
    int failed = 0;
    auto check = [&](bool ok, const char* name) {
        cout << (ok ? "PASS " : "FAIL ") << name << endl;
        if (!ok) failed++;
    };

    DynArray<int> values;
    values.push(90);
    values.push(72);
    values.push(84);
    values.push(31);
    values.push(95);
    values.push(79);
    values.push(77);

    HeapBuildLevel level;
    level.load(values);
    check(!level.isComplete() && !level.isFailed(),
          "heap build level starts with a broken raw heap");

    bool accepted = true;
    EventLog invalid = level.chooseSwap(0, 2, accepted);
    check(!accepted && invalid.empty() && level.score().alarm() == 1,
          "invalid heap swap adds one alarm penalty");

    check(level.isValidSwap(1, 4) && level.isValidSwap(4, 1),
          "valid heap swap accepts parent child selection in either order");

    EventLog first = level.chooseSwap(1, 4, accepted);
    check(accepted && first.size() == 2 && level.valueAt(1) == 95 &&
          first.at(0).extra == HeapReason::PLAYER,
          "valid player swap changes the model and records its reason");

    EventLog second = level.chooseSwap(0, 1, accepted);
    check(accepted && !second.empty() && level.isComplete() &&
          level.score().isFinished() && level.score().steps() == 2,
          "required swaps complete the heap build level");

    DynArray<int> copy;
    level.copyValues(copy);
    int modelRoot = level.valueAt(0);
    copy[0] = -1;
    check(level.valueAt(0) == modelRoot,
          "heap build snapshot does not change the model");

    return failed;
}
