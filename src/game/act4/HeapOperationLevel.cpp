#include "game/act4/HeapOperationLevel.h"

namespace vh {

HeapOperationLevel::HeapOperationLevel() : scoring(1), complete(false) {}

void HeapOperationLevel::load(const DynArray<int>& values, const std::string& task) {
    heap.load(values);
    heap.buildMaxHeap();
    taskName = task;
    history.clear();
    int optimal = task == "heap_sort" ? 1 : values.size();
    scoring.reset(optimal);
    complete = values.empty();
    if (complete) scoring.finish();
}

EventLog HeapOperationLevel::perform() {
    if (complete || scoring.isFailed()) return EventLog();
    history.push(heap.serialize());
    scoring.addStep();

    EventLog events;
    if (taskName == "heap_sort") {
        events = heap.heapSort();
    } else {
        int maximum = 0;
        events = heap.extractMax(maximum);
    }

    complete = heap.activeSize() == 0 && heap.isSortedAscending();
    if (complete) scoring.finish();
    return events;
}

bool HeapOperationLevel::undo() {
    if (history.empty()) return false;
    bool restored = heap.deserialize(history.pop());
    if (restored) complete = false;
    return restored;
}

bool HeapOperationLevel::isComplete() const { return complete; }
bool HeapOperationLevel::isFailed() const { return scoring.isFailed(); }
int HeapOperationLevel::activeSize() const { return heap.activeSize(); }

void HeapOperationLevel::copyValues(DynArray<int>& output) const {
    output.clear();
    for (int index = 0; index < heap.size(); index++) output.push(heap.at(index));
}

const ScoreSystem& HeapOperationLevel::score() const { return scoring; }

}
