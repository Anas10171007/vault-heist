#include <iostream>
#include <string>

#include "game/LevelManager.h"
#include "game/LevelPlay.h"
#include "game/LevelPlayFactory.h"

using namespace std;
using namespace vh;

namespace {

bool sameScore(const ScoreSystem& first, const ScoreSystem& second) {
    return first.steps() == second.steps() && first.alarm() == second.alarm() &&
           first.alarmCapacity() == second.alarmCapacity() && first.penalties() == second.penalties() &&
           first.score() == second.score() && first.stars() == second.stars() &&
           first.usedHint() == second.usedHint() && first.isFailed() == second.isFailed() &&
           first.isFinished() == second.isFinished();
}

bool sameAction(const PlayerAction& first, const PlayerAction& second) {
    return first.kind == second.kind && first.id == second.id && first.value == second.value &&
           first.slot == second.slot && first.choice == second.choice;
}

bool sameTreeData(const TreeSnapshot& a, const TreeSnapshot& b) {
    if (a.rootId != b.rootId || a.ids.size() != b.ids.size()) return false;
    for (int i = 0; i < a.ids.size(); i++) {
        if (a.ids[i] != b.ids[i] || a.values[i] != b.values[i] ||
            a.leftIds[i] != b.leftIds[i] || a.rightIds[i] != b.rightIds[i] ||
            a.extraA[i] != b.extraA[i] || a.extraB[i] != b.extraB[i]) {
            return false;
        }
    }
    return true;
}

bool sameTree(const BinaryTree& first, const TreeSnapshot& second) {
    return sameTreeData(first.captureTree(), second);
}

bool snapshotRoundTrip(LevelPlay& play, int actionsToPlay, bool includeHint, bool includeWrong) {
    LevelSnapshot saved = play.captureSnapshot();
    TreeSnapshot savedTree = play.tree().captureTree();
    const ScoreSystem before = play.score();
    PlayerAction next = play.nextAction();
    bool complete = play.isComplete();
    bool failed = play.isFailed();

    for (int i = 0; i < actionsToPlay && !play.isComplete() && !play.isFailed(); i++) {
        play.apply(play.nextAction());
    }
    if (includeHint && !play.isComplete() && !play.isFailed()) play.hint();
    if (includeWrong && !play.isComplete() && !play.isFailed()) play.apply(clickNode(-999));

    play.restoreSnapshot(saved);
    return sameTree(play.tree(), savedTree) && sameScore(play.score(), before) &&
           sameAction(play.nextAction(), next) && play.isComplete() == complete &&
           play.isFailed() == failed;
}

}

int runLevelSnapshotTests() {
    int failed = 0;
    auto check = [&](bool ok, const char* name) {
        cout << (ok ? "PASS " : "FAIL ") << name << endl;
        if (!ok) failed++;
    };

    LevelManager manager;
    bool loaded = manager.loadAll("data/levels");
    check(loaded && manager.size() == 20, "snapshot suite loads all four act files");
    if (!loaded) return failed;

    bool allRoundTrips = true;
    bool allWrongRestored = true;
    bool allHintRestored = true;
    bool allRestarts = true;
    bool allWinRestored = true;
    for (int i = 0; i < 15; i++) {
        const LevelDef& def = manager.at(i);
        LevelPlay* play = createLevelPlay(def);
        if (play == nullptr || !play->load(def)) {
            allRoundTrips = false;
            delete play;
            continue;
        }
        play->restart();
        if (!snapshotRoundTrip(*play, 2, false, false)) allRoundTrips = false;
        if (!snapshotRoundTrip(*play, 1, true, false)) allHintRestored = false;
        if (!snapshotRoundTrip(*play, 1, false, true)) allWrongRestored = false;

        LevelSnapshot beforeWin = play->captureSnapshot();
        int guard = 0;
        while (!play->isComplete() && !play->isFailed() && guard < 100) {
            if (!play->apply(play->nextAction()).correct) break;
            guard++;
        }
        bool won = play->isComplete() && !play->isFailed() && play->score().stars() == 3;
        play->restoreSnapshot(beforeWin);
        if (play->isComplete() || play->score().steps() != 0) allWinRestored = false;
        if (!won) allWinRestored = false;

        LevelSnapshot afterLoad = play->captureSnapshot();
        play->restart();
        if (play->captureSnapshot().actions.size() != 0) allRestarts = false;
        play->restoreSnapshot(afterLoad);
        if (play->captureSnapshot().actions.size() != afterLoad.actions.size()) allRestarts = false;
        delete play;
    }
    check(allRoundTrips, "snapshot restores tree, score and next action on every level");
    check(allHintRestored, "snapshot restores hint state on every level");
    check(allWrongRestored, "undo after a wrong action restores alarm and penalties");
    check(allWinRestored, "restoring a completed snapshot clears the win state");
    check(allRestarts, "restart clears history and restore rebuilds it");

    const LevelDef* def = manager.load("1.1");
    LevelPlay* play = createLevelPlay(*def);
    play->load(*def);
    play->restart();
    play->apply(play->nextAction());
    LevelSnapshot firstStep = play->captureSnapshot();
    play->apply(play->nextAction());
    play->apply(play->nextAction());
    play->restoreSnapshot(firstStep);
    check(play->score().steps() == 1 && play->captureSnapshot().actions.size() == 1,
          "undo twice returns exactly to the saved step count");
    delete play;

    return failed;
}
