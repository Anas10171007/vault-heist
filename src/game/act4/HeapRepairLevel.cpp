#include "game/act4/HeapRepairLevel.h"

namespace vh {

HeapRepairLevel::HeapRepairLevel() : scoring(1), complete(false) {}

void HeapRepairLevel::load(const DynArray<int>& values) {
    heap.load(values);
    scoring.reset(1);
    complete = heap.isMaxHeap();
    if (complete) scoring.finish();
}

bool HeapRepairLevel::needsUp(int index) const {
    if (index <= 0 || index >= heap.activeSize()) return false;
    int parent = (index - 1) / 2;
    return heap.at(index) > heap.at(parent);
}

bool HeapRepairLevel::needsDown(int index) const {
    if (index < 0 || index >= heap.activeSize()) return false;
    int left = index * 2 + 1;
    int right = index * 2 + 2;
    if (left < heap.activeSize() && heap.at(left) > heap.at(index)) return true;
    if (right < heap.activeSize() && heap.at(right) > heap.at(index)) return true;
    return false;
}

EventLog HeapRepairLevel::chooseRepair(int index, int direction, bool& accepted) {
    accepted = false;
    if (complete || scoring.isFailed()) return EventLog();

    bool valid = direction == HeapDirection::UP ? needsUp(index) : needsDown(index);
    if (!valid) {
        scoring.addPenalty(PenaltyKind::INVALID_HEAP_ACTION);
        return EventLog();
    }

    accepted = true;
    scoring.addStep();
    EventLog events = direction == HeapDirection::UP ? heap.repairUp(index)
                                                     : heap.repairDown(index);
    if (heap.isMaxHeap()) {
        complete = true;
        scoring.finish();
    }
    return events;
}

bool HeapRepairLevel::isComplete() const { return complete; }
bool HeapRepairLevel::isFailed() const { return scoring.isFailed(); }
int HeapRepairLevel::size() const { return heap.size(); }
int HeapRepairLevel::valueAt(int index) const { return heap.at(index); }

void HeapRepairLevel::copyValues(DynArray<int>& output) const {
    output.clear();
    for (int index = 0; index < heap.size(); index++) output.push(heap.at(index));
}

const ScoreSystem& HeapRepairLevel::score() const { return scoring; }

}
