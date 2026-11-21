#pragma once
#include <string>
#include "game/ScoreSystem.h"
#include "model/heap/MaxHeap.h"
#include "util/DynArray.h"

using namespace std;

namespace vh {

struct HeapDirection {
    static constexpr int UP = 0;
    static constexpr int DOWN = 1;
};

class HeapRepairLevel {
public:
    HeapRepairLevel();

    void load(const DynArray<int>& values);
    EventLog chooseRepair(int index, int direction, bool& accepted);
    bool undo();
    bool needsUp(int index) const;
    bool needsDown(int index) const;
    bool isComplete() const;
    bool isFailed() const;
    int size() const;
    int valueAt(int index) const;
    void copyValues(DynArray<int>& output) const;
    const ScoreSystem& score() const;

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
};

}
