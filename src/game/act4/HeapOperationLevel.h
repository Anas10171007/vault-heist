#pragma once
#include <string>
#include "game/ScoreSystem.h"
#include "model/heap/MaxHeap.h"
#include "util/DynArray.h"

using namespace std;

namespace vh {

class HeapOperationLevel {
public:
    HeapOperationLevel();

    void load(const DynArray<int>& values, const string& task, int targetExtractions = 0);
    EventLog extractMaximum();
    EventLog chooseSwap(int first, int second, bool& accepted);
    bool undo();
    bool isComplete() const;
    bool isFailed() const;
    bool needsRestore() const;
    int activeSize() const;
    void copyValues(DynArray<int>& output) const;
    const ScoreSystem& score() const;

private:
    struct Snapshot {
        string heapState;
        ScoreSystem scoreState;
        bool wasComplete;
        int extractionsDone;
        bool awaitingRestore;
    };

    MaxHeap heap;
    ScoreSystem scoring;
    string taskName;
    DynArray<Snapshot> history;
    int extractionGoal;
    int completedExtractions;
    bool awaitingRestore;
    bool complete;

    void pushSnapshot();
    void refreshComplete();
    bool isValidSwap(int first, int second) const;
    int countOptimalSteps(const DynArray<int>& values) const;
};

}
