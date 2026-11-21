#include "game/act4/HeapOperationLevel.h"

using namespace std;

namespace vh {

HeapOperationLevel::HeapOperationLevel()
    : scoring(1), extractionGoal(0), completedExtractions(0),
      awaitingRestore(false), awaitingExtractConfirm(false), awaitingBuild(false),
      complete(false) {}

int HeapOperationLevel::countOptimalSteps(const DynArray<int>& values) const {
    MaxHeap solution(false);
    solution.load(values);

    int steps = 0;
    if (taskName == "heap_sort" && !solution.isMaxHeap()) {
        steps += 1; // one Build Max Heap player action
    }
    solution.buildMaxHeap();

    int goal = extractionGoal;
    for (int extract = 0; extract < goal; extract++) {
        if (solution.activeSize() <= 0) break;

        int maximum = 0;
        solution.extractMaxNoHeapify(maximum);
        steps++;

        if (solution.activeSize() > 0 && !solution.isMaxHeap()) {
            int swapsBefore = solution.swaps();
            solution.repairDown(0);
            steps += solution.swaps() - swapsBefore;
        }
    }

    if (steps < 1) steps = 1;
    return steps;
}

void HeapOperationLevel::load(const DynArray<int>& values, const string& task,
                              int targetExtractions) {
    heap.load(values);
    taskName = task;
    history.clear();
    extractionGoal = targetExtractions;
    if (task == "heap_race") {
        if (extractionGoal <= 0) extractionGoal = values.size();
    } else {
        extractionGoal = values.size();
    }

    awaitingBuild = false;
    if (task == "heap_sort") {
        awaitingBuild = !heap.isMaxHeap();
    } else {
        heap.buildMaxHeap();
    }

    completedExtractions = 0;
    awaitingRestore = false;
    awaitingExtractConfirm = false;
    scoring.reset(countOptimalSteps(values));

    // Race: tighter alarm than the default 1.5×opt+2 ceiling.
    if (task == "heap_race") {
        int tight = scoring.optimalSteps() + extractionGoal + 1;
        scoring.setAlarmCapacity(tight);
    }

    complete = values.empty();
    if (complete) scoring.finish();
}

void HeapOperationLevel::pushSnapshot() {
    Snapshot snapshot;
    snapshot.heapState = heap.serialize();
    snapshot.scoreState = scoring;
    snapshot.wasComplete = complete;
    snapshot.extractionsDone = completedExtractions;
    snapshot.awaitingRestore = awaitingRestore;
    snapshot.awaitingExtractConfirm = awaitingExtractConfirm;
    snapshot.awaitingBuild = awaitingBuild;
    history.push(snapshot);
}

void HeapOperationLevel::refreshComplete() {
    if (awaitingRestore || awaitingExtractConfirm || awaitingBuild) {
        complete = false;
        return;
    }

    if (taskName == "heap_race") {
        complete = completedExtractions >= extractionGoal;
    } else {
        complete = heap.activeSize() == 0 && heap.isSortedAscending();
    }

    if (complete) scoring.finish();
}

bool HeapOperationLevel::isValidSwap(int first, int second) const {
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

bool HeapOperationLevel::isValidExtractPair(int first, int second) const {
    int last = lastActiveIndex();
    if (last < 0) return false;
    if (last == 0) return first == 0 && second == 0;
    return (first == 0 && second == last) || (second == 0 && first == last);
}

EventLog HeapOperationLevel::runBuildMaxHeap(bool& accepted) {
    accepted = false;
    if (complete || scoring.isFailed()) return EventLog();
    if (!awaitingBuild) {
        scoring.addPenalty(PenaltyKind::INVALID_HEAP_ACTION);
        return EventLog();
    }

    accepted = true;
    pushSnapshot();
    scoring.addStep();
    EventLog events = heap.buildMaxHeap();
    awaitingBuild = false;
    refreshComplete();
    return events;
}

EventLog HeapOperationLevel::runHeapifyDownRoot(bool& accepted) {
    accepted = false;
    if (complete || scoring.isFailed()) return EventLog();
    if (awaitingBuild || awaitingExtractConfirm || !awaitingRestore) {
        scoring.addPenalty(PenaltyKind::INVALID_HEAP_ACTION);
        return EventLog();
    }
    if (heap.activeSize() <= 0) {
        scoring.addPenalty(PenaltyKind::INVALID_HEAP_ACTION);
        return EventLog();
    }

    accepted = true;
    pushSnapshot();
    scoring.addStep();
    EventLog events = heap.repairDown(0);
    if (heap.isMaxHeap()) awaitingRestore = false;
    refreshComplete();
    return events;
}

EventLog HeapOperationLevel::chooseExtract(int first, int second, bool& accepted) {
    accepted = false;
    if (complete || scoring.isFailed()) return EventLog();

    if (awaitingBuild || awaitingExtractConfirm || awaitingRestore || !heap.isMaxHeap() ||
        heap.activeSize() <= 0) {
        scoring.addPenalty(PenaltyKind::INVALID_HEAP_ACTION);
        return EventLog();
    }

    if (!isValidExtractPair(first, second)) {
        scoring.addPenalty(PenaltyKind::INVALID_HEAP_ACTION);
        return EventLog();
    }

    accepted = true;
    pushSnapshot();

    EventLog events;
    int last = lastActiveIndex();
    if (last > 0) {
        events = heap.playerSwap(0, last);
    } else {
        events.append(Event(EventType::DONE));
    }
    awaitingExtractConfirm = true;
    refreshComplete();
    return events;
}

EventLog HeapOperationLevel::confirmExtract(bool& accepted) {
    accepted = false;
    if (complete || scoring.isFailed()) return EventLog();

    if (!awaitingExtractConfirm || heap.activeSize() <= 0) {
        scoring.addPenalty(PenaltyKind::INVALID_HEAP_ACTION);
        return EventLog();
    }

    accepted = true;
    pushSnapshot();
    scoring.addStep();

    int sealed = 0;
    EventLog events = heap.sealLastSorted(sealed);
    completedExtractions++;
    awaitingExtractConfirm = false;
    awaitingRestore = heap.activeSize() > 0 && !heap.isMaxHeap();

    if (taskName == "heap_sort" && heap.activeSize() == 0) {
        events.append(Event(EventType::HEAP_SORT_COMPLETE, heap.size()));
    }
    events.append(Event(EventType::DONE));

    refreshComplete();
    return events;
}

EventLog HeapOperationLevel::chooseSwap(int first, int second, bool& accepted) {
    accepted = false;
    if (complete || scoring.isFailed()) return EventLog();

    if (awaitingBuild || awaitingExtractConfirm || !awaitingRestore) {
        scoring.addPenalty(PenaltyKind::INVALID_HEAP_ACTION);
        return EventLog();
    }

    if (!isValidSwap(first, second)) {
        scoring.addPenalty(PenaltyKind::INVALID_HEAP_ACTION);
        return EventLog();
    }

    accepted = true;
    pushSnapshot();
    scoring.addStep();
    EventLog events = heap.playerSwap(first, second);
    if (heap.isMaxHeap()) awaitingRestore = false;
    refreshComplete();
    return events;
}

bool HeapOperationLevel::undo() {
    if (history.empty()) return false;
    Snapshot snapshot = history.pop();
    if (!heap.deserialize(snapshot.heapState)) return false;
    scoring = snapshot.scoreState;
    complete = snapshot.wasComplete;
    completedExtractions = snapshot.extractionsDone;
    awaitingRestore = snapshot.awaitingRestore;
    awaitingExtractConfirm = snapshot.awaitingExtractConfirm;
    awaitingBuild = snapshot.awaitingBuild;
    return true;
}

bool HeapOperationLevel::isComplete() const { return complete; }
bool HeapOperationLevel::isFailed() const { return scoring.isFailed(); }
bool HeapOperationLevel::needsRestore() const { return awaitingRestore; }
bool HeapOperationLevel::needsExtractConfirm() const { return awaitingExtractConfirm; }
bool HeapOperationLevel::needsBuild() const { return awaitingBuild; }
int HeapOperationLevel::activeSize() const { return heap.activeSize(); }
int HeapOperationLevel::lastActiveIndex() const {
    return heap.activeSize() > 0 ? heap.activeSize() - 1 : -1;
}

void HeapOperationLevel::copyValues(DynArray<int>& output) const {
    output.clear();
    for (int index = 0; index < heap.size(); index++) output.push(heap.at(index));
}

const ScoreSystem& HeapOperationLevel::score() const { return scoring; }

void HeapOperationLevel::applyHint() { scoring.addPenalty(PenaltyKind::HINT); }

}
