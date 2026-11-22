#include "game/act4/HeapRepairLevel.h"

using namespace std;

namespace vh {

HeapRepairLevel::HeapRepairLevel() : scoring(1), complete(false) {}

void HeapRepairLevel::load(const DynArray<int>& values) {
    heap.load(values);
    history.clear();
    MaxHeap solution(false);
    solution.load(values);
    int before = solution.swaps();
    solution.repairDown(0);
    int opt = solution.swaps() - before;
    if (opt < 1) opt = 1;
    scoring.reset(opt);
    complete = heap.isMaxHeap();
    if (complete) scoring.finish();
}

void HeapRepairLevel::pushSnapshot() {
    Snapshot snapshot;
    snapshot.heapState = heap.serialize();
    snapshot.scoreState = scoring;
    snapshot.wasComplete = complete;
    history.push(snapshot);
}

bool HeapRepairLevel::canSiftUp(int index) const {
    if (index <= 0 || index >= heap.activeSize()) return false;
    int parent = (index - 1) / 2;
    return heap.at(index) > heap.at(parent);
}

bool HeapRepairLevel::canSiftDown(int index) const {
    if (index < 0 || index >= heap.activeSize()) return false;
    int left = index * 2 + 1;
    int right = index * 2 + 2;
    if (left >= heap.activeSize()) return false;
    if (heap.at(left) > heap.at(index)) return true;
    if (right < heap.activeSize() && heap.at(right) > heap.at(index)) return true;
    return false;
}

EventLog HeapRepairLevel::chooseHeapify(int index, bool siftUp, bool& accepted) {
    accepted = false;
    if (complete || scoring.isFailed()) return EventLog();

    bool valid = siftUp ? canSiftUp(index) : canSiftDown(index);
    if (!valid) {
        scoring.addPenalty(PenaltyKind::INVALID_HEAP_ACTION);
        return EventLog();
    }

    pushSnapshot();
    accepted = true;
    scoring.addStep();
    EventLog events = siftUp ? heap.repairUp(index) : heap.repairDown(index);
    if (heap.isMaxHeap()) {
        complete = true;
        scoring.finish();
    }
    return events;
}

bool HeapRepairLevel::undo() {
    if (history.empty()) return false;
    Snapshot snapshot = history.pop();
    if (!heap.deserialize(snapshot.heapState)) return false;
    scoring = snapshot.scoreState;
    complete = snapshot.wasComplete;
    return true;
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

void HeapRepairLevel::applyHint() { scoring.addPenalty(PenaltyKind::HINT); }

}
