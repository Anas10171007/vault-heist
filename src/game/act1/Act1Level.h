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
    bool undo() override;
    PlayerAction nextAction() const override;
    int optimalSteps() const override;
    bool isComplete() const override;
    bool isFailed() const override;
    const ScoreSystem& score() const override;
    const BinaryTree& tree() const override;
    const LevelDef& definition() const override;

private:
    void buildInitialTree();
    void computePlan();
    bool sameAction(const PlayerAction& first, const PlayerAction& second) const;
    PlayResult finish(const char* message, EventLog& events);
    PlayResult reject(const char* message);
    PlayResult accept(const char* message, EventLog& events);
    bool placementCompletesTree() const;
    void pushSnapshot();

    struct Snapshot {
        std::string treeState;
        std::string targetState;
        ScoreSystem scoreState;
        DynArray<int> expectedState;
        DynArray<int> placedState;
        int firstValue = 0;
        int secondValue = 0;
        int answerOne = 0;
        int answerTwo = 0;
        int progress = 0;
        int stage = 0;
        int optimal = 1;
        bool complete = false;
    };

    LevelDef definitionData;
    BinaryTree treeModel;
    BinaryTree targetTree;
    ScoreSystem scoring;
    DynArray<int> expected;
    DynArray<int> placed;
    DynArray<Snapshot> history;
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
