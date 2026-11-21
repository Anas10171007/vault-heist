#pragma once

#include "game/LevelPlay.h"
#include "model/tree/AVLTree.h"

namespace vh {

class Act3Level : public LevelPlay {
public:
    Act3Level();

    bool load(const LevelDef& definition) override;
    void reset() override;
    PlayResult apply(const PlayerAction& action) override;
    PlayResult hint() override;
    bool undo() override;
    PlayerAction nextAction() const override;
    int optimalSteps() const override;
    bool isComplete() const override;
    bool isFailed() const override;
    const ScoreSystem& score() const override;
    const BinaryTree& tree() const override;
    const LevelDef& definition() const override;

private:
    struct Repair {
        bool found = false;
        int nodeId = 0;
        int pivotId = 0;
        int caseType = 0;
    };

    void buildInitialTree();
    void computePlan();
    Repair findRepair(const AVLTree& tree) const;
    void applyRepair(const Repair& repair, EventLog& events);
    int predictedRotation(int key) const;
    void prepareKthStage();
    bool sameAction(const PlayerAction& first, const PlayerAction& second) const;
    PlayResult finish(const char* message, EventLog& events);
    PlayResult reject(const char* message, bool rotation);
    PlayResult accept(const char* message, EventLog& events);
    void pushSnapshot();

    struct Snapshot {
        std::string treeState;
        ScoreSystem scoreState;
        DynArray<int> expectedState;
        DynArray<int> insertsState;
        Repair repairState;
        int progress = 0;
        int k = 0;
        int expectedSize = 0;
        int stage = 0;
        int stages = 1;
        int phase = 0;
        int insertIndex = 0;
        int optimal = 1;
        bool complete = false;
    };

    LevelDef definitionData;
    AVLTree treeModel;
    ScoreSystem scoring;
    DynArray<int> expected;
    DynArray<int> inserts;
    DynArray<Snapshot> history;
    Repair repair;
    int progress = 0;
    int k = 0;
    int expectedSize = 0;
    int stage = 0;
    int stages = 1;
    int phase = 0;
    int insertIndex = 0;
    int optimal = 1;
    bool complete = false;
};

}
