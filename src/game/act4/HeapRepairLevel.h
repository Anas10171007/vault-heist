#pragma once
#include <string>
#include "game/ScoreSystem.h"
#include "model/heap/MaxHeap.h"
#include "util/DynArray.h"

using namespace std;

namespace vh {

class HeapRepairLevel {
public:
    HeapRepairLevel();

    void load(const DynArray<int>& values);
    // siftUp = true → repairUp; false → repairDown.
    EventLog chooseHeapify(int index, bool siftUp, bool& accepted);
    bool canSiftUp(int index) const;
    bool canSiftDown(int index) const;
    bool undo();
    bool isComplete() const;
    bool isFailed() const;
    int size() const;
    int valueAt(int index) const;
    void copyValues(DynArray<int>& output) const;
    const ScoreSystem& score() const;
    void applyHint();

private:
    struct Snapshot {
        string heapState;
        ScoreSystem scoreState;
        bool wasComplete;
    };

    MaxHeap heap;
    ScoreSystem scoring;
    DynArray<Snapshot> history;
    bool complete;

    void pushSnapshot();
};

}
