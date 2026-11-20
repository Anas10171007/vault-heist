#include "game/act4/HeapBuildLevel.h"

namespace vh {

HeapBuildLevel::HeapBuildLevel() : scoring(1), complete(false) {}

void HeapBuildLevel::load(const DynArray<int>& values) {
    heap.load(values);
    MaxHeap solution(false);
    solution.load(values);
    solution.buildMaxHeap();
    scoring.reset(solution.swaps());
    complete = heap.isMaxHeap();
    if (complete) scoring.finish();
}

bool HeapBuildLevel::isValidSwap(int first, int second) const {
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

EventLog HeapBuildLevel::chooseSwap(int first, int second, bool& accepted) {
    accepted = false;
    if (complete || scoring.isFailed()) return EventLog();

    if (!isValidSwap(first, second)) {
        scoring.addPenalty(PenaltyKind::INVALID_HEAP_ACTION);
        return EventLog();
    }

    accepted = true;
    scoring.addStep();
    EventLog events = heap.playerSwap(first, second);
    if (heap.isMaxHeap()) {
        complete = true;
        scoring.finish();
    }
    return events;
}

bool HeapBuildLevel::isComplete() const { return complete; }
bool HeapBuildLevel::isFailed() const { return scoring.isFailed(); }
int HeapBuildLevel::size() const { return heap.size(); }
int HeapBuildLevel::valueAt(int index) const { return heap.at(index); }

void HeapBuildLevel::copyValues(DynArray<int>& output) const {
    output.clear();
    for (int index = 0; index < heap.size(); index++) output.push(heap.at(index));
}

const ScoreSystem& HeapBuildLevel::score() const { return scoring; }

}
