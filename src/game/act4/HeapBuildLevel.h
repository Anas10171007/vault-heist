#pragma once
#include <string>
#include "game/ScoreSystem.h"
#include "model/heap/MaxHeap.h"
#include "util/DynArray.h"

using namespace std;

namespace vh {

class HeapBuildLevel {
public:
    HeapBuildLevel();

    void load(const DynArray<int>& values);
    EventLog chooseSwap(int first, int second, bool& accepted);
    bool undo();
    bool isValidSwap(int first, int second) const;
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
