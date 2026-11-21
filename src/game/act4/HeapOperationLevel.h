#pragma once
#include <string>
#include "game/ScoreSystem.h"
#include "model/heap/MaxHeap.h"
#include "util/DynArray.h"

namespace vh {

class HeapOperationLevel {
public:
    HeapOperationLevel();

    void load(const DynArray<int>& values, const std::string& task);
    EventLog perform();
    bool undo();
    bool isComplete() const;
    bool isFailed() const;
    int activeSize() const;
    void copyValues(DynArray<int>& output) const;
    const ScoreSystem& score() const;

private:
    MaxHeap heap;
    ScoreSystem scoring;
    std::string taskName;
    DynArray<std::string> history;
    bool complete;
};

}
