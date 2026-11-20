#pragma once
#include "game/ScoreSystem.h"
#include "model/heap/MaxHeap.h"

namespace vh {

class HeapBuildLevel {
public:
    HeapBuildLevel();

    void load(const DynArray<int>& values);
    EventLog chooseSwap(int first, int second, bool& accepted);
    bool isValidSwap(int first, int second) const;
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
