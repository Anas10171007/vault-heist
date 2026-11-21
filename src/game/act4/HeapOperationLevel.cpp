#include "game/act4/HeapOperationLevel.h"

using namespace std;

namespace vh {

HeapOperationLevel::HeapOperationLevel()
    : scoring(1), extractionGoal(0), completedExtractions(0),
      awaitingHeapify(false), complete(false) {}

void HeapOperationLevel::load(const DynArray<int>& values, const string& task,
                              int targetExtractions) {
    heap.load(values);
    heap.buildMaxHeap();
    taskName = task;
    history.clear();
    extractionGoal = targetExtractions;
    if (task == "heap_race") {
        if (extractionGoal <= 0) extractionGoal = values.size();
    } else {
        extractionGoal = values.size();
    }
    completedExtractions = 0;
    awaitingHeapify = false;

    // Each extract is one step. Each restore is one step. The last extract that
    // empties the heap needs no restore, so a full clear costs 2n - 1 steps.
    int restores = extractionGoal;
    if (extractionGoal > 0 && extractionGoal >= values.size()) restores = extractionGoal - 1;
    int optimal = extractionGoal + restores;
    if (optimal < 1) optimal = 1;
    scoring.reset(optimal);

    complete = values.empty();
    if (complete) scoring.finish();
}

void HeapOperationLevel::pushSnapshot() {
    Snapshot snapshot;
    snapshot.heapState = heap.serialize();
    snapshot.scoreState = scoring;
    snapshot.wasComplete = complete;
    snapshot.extractionsDone = completedExtractions;
    snapshot.awaitingHeapify = awaitingHeapify;
    history.push(snapshot);
}

void HeapOperationLevel::refreshComplete() {
    if (awaitingHeapify) {
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

EventLog HeapOperationLevel::extractMaximum() {
    if (complete || scoring.isFailed()) return EventLog();

    if (awaitingHeapify || heap.activeSize() <= 0) {
        scoring.addPenalty(PenaltyKind::INVALID_HEAP_ACTION);
        return EventLog();
    }

    pushSnapshot();
    scoring.addStep();

    int maximum = 0;
    EventLog events = heap.extractMaxNoHeapify(maximum);
    completedExtractions++;
    awaitingHeapify = heap.activeSize() > 0;

    if (taskName == "heap_sort" && heap.activeSize() == 0) {
        events.append(Event(EventType::HEAP_SORT_COMPLETE, heap.size()));
    }
    events.append(Event(EventType::DONE));

    refreshComplete();
    return events;
}

EventLog HeapOperationLevel::restoreHeap() {
    if (complete || scoring.isFailed()) return EventLog();

    if (!awaitingHeapify) {
        scoring.addPenalty(PenaltyKind::INVALID_HEAP_ACTION);
        return EventLog();
    }

    pushSnapshot();
    scoring.addStep();

    EventLog events = heap.repairDown(0);
    awaitingHeapify = false;
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
    awaitingHeapify = snapshot.awaitingHeapify;
    return true;
}

bool HeapOperationLevel::isComplete() const { return complete; }
bool HeapOperationLevel::isFailed() const { return scoring.isFailed(); }
bool HeapOperationLevel::needsHeapify() const { return awaitingHeapify; }
int HeapOperationLevel::activeSize() const { return heap.activeSize(); }

void HeapOperationLevel::copyValues(DynArray<int>& output) const {
    output.clear();
    for (int index = 0; index < heap.size(); index++) output.push(heap.at(index));
}

const ScoreSystem& HeapOperationLevel::score() const { return scoring; }

}
