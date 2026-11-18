#include <iostream>
#include "gui/widgets/HeapView.h"
#include "model/heap/MaxHeap.h"

using namespace std;
using namespace vh;

int runHeapViewTests() {
    int failed = 0;
    auto check = [&](bool ok, const char* name) {
        cout << (ok ? "PASS " : "FAIL ") << name << endl;
        if (!ok) failed++;
    };

    DynArray<int> start;
    start.push(90);
    start.push(70);
    start.push(80);
    start.push(20);
    start.push(60);

    HeapView view;
    view.load(start, 5);
    check(view.valueCount() == 5 && view.activeSize() == 5,
          "heap view loads one shared visual state");

    view.onEvent(Event(EventType::HEAP_SWAP, 0, 4), 0.5f);
    check(view.valueAt(0) == 90 && view.valueAt(4) == 60 &&
          view.swappingFirst() == 0 && view.swappingSecond() == 4,
          "heap view animates a swap before changing values");

    view.onEvent(Event(EventType::HEAP_SWAP, 0, 4), 1.f);
    check(view.valueAt(0) == 60 && view.valueAt(4) == 90 &&
          view.swappingFirst() == -1,
          "heap view completes a swap once");

    view.onEvent(Event(EventType::HEAP_SIZE_CHANGE, 0, 0, Side::NONE,
                       0, 5, 4), 1.f);
    check(view.activeSize() == 4,
          "heap size change moves the array boundary");

    check(view.isViolation(0) && view.isViolation(2),
          "heap view flags both ends of a broken parent child pair");

    view.reset();
    check(view.valueAt(0) == 90 && view.valueAt(4) == 60 &&
          view.activeSize() == 5,
          "heap view reset restores the starting snapshot");

    view.onEvent(Event(EventType::INSERT, 5, 0, Side::NONE, 95), 1.f);
    view.onEvent(Event(EventType::HEAP_SIZE_CHANGE, 0, 0, Side::NONE,
                       0, 5, 6), 1.f);
    check(view.valueCount() == 6 && view.valueAt(5) == 95 &&
          view.activeSize() == 6,
          "heap view follows insertion events");

    DynArray<int> raw;
    raw.push(3);
    raw.push(9);
    raw.push(2);
    raw.push(7);
    raw.push(5);
    raw.push(8);
    MaxHeap heap;
    heap.load(raw);
    EventLog sortEvents = heap.heapSort();
    HeapView replay;
    replay.load(raw, raw.size());
    AnimationController animation;
    animation.setListener(&replay);
    animation.load(sortEvents);
    animation.finishInstantly();
    bool ascending = true;
    for (int index = 1; index < replay.valueCount(); index++) {
        if (replay.valueAt(index - 1) > replay.valueAt(index)) ascending = false;
    }
    check(replay.activeSize() == 0 && ascending,
          "full Heap Sort replay keeps tree and array state synchronized");

    return failed;
}
