#pragma once

#include "game/LevelPlay.h"
#include "model/tree/BinaryTree.h"

namespace vh {

class Act1Level : public LevelPlay {
public:
    Act1Level();

    bool load(const LevelDef& definition) override;
    void reset() override;
    PlayResult apply(const PlayerAction& action) override;
    PlayResult hint() override;
    LevelSnapshot captureSnapshot() const override;
    void restoreSnapshot(const LevelSnapshot& snapshot) override;
    PlayerAction nextAction() const override;
    int optimalSteps() const override;
    bool isComplete() const override;
    bool isFailed() const override;
    const ScoreSystem& score() const override;
    const BinaryTree& tree() const override;
    const LevelDef& definition() const override;

private:
    void restoreModelTree(const TreeSnapshot& snapshot) override;
    void restoreScore(const ScoreSnapshot& snapshot) override;

    void buildInitialTree();
    void computePlan();
    bool sameAction(const PlayerAction& first, const PlayerAction& second) const;
    PlayResult finish(const char* message, EventLog& events);
    PlayResult reject(const char* message);
    PlayResult accept(const char* message, EventLog& events);
    bool placementCompletesTree() const;

    LevelDef definitionData;
    BinaryTree treeModel;
    BinaryTree targetTree;
    ScoreSystem scoring;
    DynArray<int> expected;
    DynArray<int> placed;
    int firstValue = 0;
    int secondValue = 0;
    int answerOne = 0;
    int answerTwo = 0;
    int progress = 0;
    int stage = 0;
    int optimal = 1;
    bool complete = false;
};

}
