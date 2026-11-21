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
    // Player picks root (0) and last active leaf: swaps only. Max stays visible
    // until confirmExtract (drag to SORTED / Extract button).
    EventLog chooseExtract(int first, int second, bool& accepted);
    EventLog confirmExtract(bool& accepted);
    EventLog chooseSwap(int first, int second, bool& accepted);
    EventLog runBuildMaxHeap(bool& accepted);
    EventLog runHeapifyDownRoot(bool& accepted);
    bool undo();
    bool isComplete() const;
    bool isFailed() const;
    bool needsRestore() const;
    bool needsExtractConfirm() const;
    bool needsBuild() const;
    int activeSize() const;
    int lastActiveIndex() const;
    void copyValues(DynArray<int>& output) const;
    const ScoreSystem& score() const;
    void applyHint();

private:
    struct Snapshot {
        string heapState;
        ScoreSystem scoreState;
        bool wasComplete;
        int extractionsDone;
        bool awaitingRestore;
        bool awaitingExtractConfirm;
        bool awaitingBuild;
    };

    MaxHeap heap;
    ScoreSystem scoring;
    string taskName;
    DynArray<Snapshot> history;
    int extractionGoal;
    int completedExtractions;
    bool awaitingRestore;
    bool awaitingExtractConfirm;
    bool awaitingBuild;
    bool complete;

    void pushSnapshot();
    void refreshComplete();
    bool isValidSwap(int first, int second) const;
    bool isValidExtractPair(int first, int second) const;
    int countOptimalSteps(const DynArray<int>& values) const;
};

}
