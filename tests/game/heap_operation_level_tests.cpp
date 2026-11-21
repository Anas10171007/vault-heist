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
    extract.perform();
    check(extract.activeSize() == 3 && !extract.isComplete(),
          "extract level advances one sorted boundary at a time");
    check(extract.undo() && extract.activeSize() == 4,
          "extract level undo restores the previous heap snapshot");
    for (int index = 0; index < 4; index++) extract.perform();
    check(extract.isComplete(), "extract level completes after every maximum is removed");

    HeapOperationLevel sort;
    sort.load(values, "heap_sort");
    EventLog events = sort.perform();
    check(sort.isComplete() && !events.empty() && sort.score().stars() == 3,
          "Heap Sort finale completes as one optimal operation");
    return failed;
}
