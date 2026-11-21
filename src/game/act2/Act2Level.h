#pragma once

#include "game/LevelPlay.h"
#include "model/tree/BST.h"

namespace vh {

class Act2Level : public LevelPlay {
public:
    Act2Level();

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
    void buildTree();
    void computePlan();
    void appendPath(int key, DynArray<int>& path) const;
    void advanceBossStage();
    bool sameAction(const PlayerAction& first, const PlayerAction& second) const;
    PlayResult finish(const char* message, EventLog& events);
    PlayResult reject(const char* message);
    PlayResult accept(const char* message, EventLog& events);
    void pushSnapshot();

    struct Snapshot {
        std::string treeState;
        ScoreSystem scoreState;
        DynArray<int> expectedState;
        DynArray<int> stageEndsState;
        int target = 0;
        int lo = 0;
        int hi = 0;
        int expectedCase = 0;
        int progress = 0;
        int stage = 0;
        int optimal = 1;
        bool complete = false;
    };

    LevelDef definitionData;
    BST treeModel;
    ScoreSystem scoring;
    DynArray<int> expected;
    DynArray<int> stageEnds;
    DynArray<Snapshot> history;
    int target = 0;
    int lo = 0;
    int hi = 0;
    int expectedCase = 0;
    int progress = 0;
    int stage = 0;
    int optimal = 1;
    bool complete = false;
};

}
