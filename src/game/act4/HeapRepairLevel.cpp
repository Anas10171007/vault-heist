#include "game/act4/HeapRepairLevel.h"

using namespace std;

namespace vh {

HeapRepairLevel::HeapRepairLevel() : scoring(1), complete(false) {}

void HeapRepairLevel::load(const DynArray<int>& values) {
    heap.load(values);
    history.clear();
    MaxHeap solution(false);
    solution.load(values);
    solution.buildMaxHeap();
    scoring.reset(solution.swaps());
    complete = heap.isMaxHeap();
    if (complete) scoring.finish();
}

bool HeapRepairLevel::isValidSwap(int first, int second) const {
    if (first < 0 || second < 0 || first >= heap.activeSize() ||
        second >= heap.activeSize() || first == second) {
        return false;
    }

    int parent = first;
    int child = second;
    if ((first - 1) / 2 == second && first > 0) {
        parent = second;
        child = first;
    } else if ((second - 1) / 2 != first || second <= 0) {
        return false;
    }

    if (heap.at(child) <= heap.at(parent)) return false;

    int left = parent * 2 + 1;
    int right = parent * 2 + 2;
    int largestChild = left;
    if (right < heap.activeSize() && heap.at(right) > heap.at(left)) {
        largestChild = right;
    }
    return child == largestChild;
}

EventLog HeapRepairLevel::chooseSwap(int first, int second, bool& accepted) {
    accepted = false;
    if (complete || scoring.isFailed()) return EventLog();

    if (!isValidSwap(first, second)) {
        scoring.addPenalty(PenaltyKind::INVALID_HEAP_ACTION);
        return EventLog();
    }

    Snapshot snapshot;
    snapshot.heapState = heap.serialize();
    snapshot.scoreState = scoring;
    snapshot.wasComplete = complete;
    history.push(snapshot);

    accepted = true;
    scoring.addStep();
    EventLog events = heap.playerSwap(first, second);
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

}
