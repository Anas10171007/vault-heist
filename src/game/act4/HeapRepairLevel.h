#pragma once
#include "game/ScoreSystem.h"
#include "model/heap/MaxHeap.h"

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
    bool needsUp(int index) const;
    bool needsDown(int index) const;
    bool isComplete() const;
    bool isFailed() const;
    int size() const;
    int valueAt(int index) const;
    void copyValues(DynArray<int>& output) const;
    const ScoreSystem& score() const;

private:
    MaxHeap heap;
    ScoreSystem scoring;
    bool complete;
};

}
