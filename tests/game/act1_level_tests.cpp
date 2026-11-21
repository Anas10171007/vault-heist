#include <iostream>
#include <string>

#include "game/LevelManager.h"
#include "game/LevelPlay.h"
#include "game/LevelPlayFactory.h"
#include "game/act1/Act1Level.h"

using namespace std;
using namespace vh;

static bool capacityMatches(const LevelPlay& level) {
    int optimal = level.optimalSteps();
    return level.score().alarmCapacity() == (optimal * 3 + 1) / 2 + 2;
}

static bool playPerfectly(LevelPlay& level, int limit) {
    int guard = 0;
    while (!level.isComplete() && !level.isFailed() && guard < limit) {
        PlayResult result = level.apply(level.nextAction());
        if (!result.correct) return false;
        guard++;
    }
    return level.isComplete() && !level.isFailed();
}

int runAct1LevelTests() {
    int failed = 0;
    auto check = [&](bool ok, const char* name) {
        cout << (ok ? "PASS " : "FAIL ") << name << endl;
        if (!ok) failed++;
    };

    LevelManager manager;
    bool loaded = manager.loadFile("data/levels/act1.json", 1);
    check(loaded && manager.size() == 5, "act 1 file loads five levels through LevelManager");
    if (!loaded) return failed;

    bool allValid = true;
    bool allOptimal = true;
    bool allCapacity = true;
    bool allWins = true;
    bool allThreeStars = true;
    for (int i = 0; i < manager.size(); i++) {
        const LevelDef& def = manager.at(i);
        LevelPlay* play = createLevelPlay(def);
        if (play == nullptr || !play->load(def)) {
            allValid = false;
            delete play;
            continue;
        }
        if (play->optimalSteps() <= 0) allOptimal = false;
        if (!capacityMatches(*play)) allCapacity = false;
        if (!playPerfectly(*play, 100)) allWins = false;
        if (play->score().stars() != 3) allThreeStars = false;
        delete play;
    }
    check(allValid, "every act 1 level creates a playable logic object");
    check(allOptimal, "every act 1 level computes a positive optimal step count");
    check(allCapacity, "every act 1 level uses the model alarm capacity formula");
    check(allWins, "scripted perfect play wins every act 1 level");
    check(allThreeStars, "scripted perfect play earns three stars on every act 1 level");

    const LevelDef* patrol = manager.load("1.1");
    Act1Level traversal;
    traversal.load(*patrol);
    int alarmBefore = traversal.score().alarm();
    int scoreBefore = traversal.score().score();
    PlayResult wrong = traversal.apply(clickNode(-99));
    check(wrong.wrong && traversal.score().alarm() == alarmBefore + 1 &&
              traversal.score().penalties() == 1 && traversal.score().score() == scoreBefore - 10,
          "wrong act 1 click adds one alarm and no flat point penalty");
    check(playPerfectly(traversal, 100), "a level still wins after a wrong click");

    Act1Level hinted;
    hinted.load(*patrol);
    PlayResult hint = hinted.hint();
    check(hint.hasHint && hinted.score().usedHint(), "hint reports its next action and is recorded");
    check(playPerfectly(hinted, 100) && hinted.score().stars() == 2,
          "a hint caps act 1 at two stars");

    Act1Level mirrorAuto;
    mirrorAuto.load(*manager.load("1.5"));
    PlayResult autoResult = mirrorAuto.apply(autoAction());
    check(autoResult.correct && mirrorAuto.isComplete() && mirrorAuto.score().usedHint(),
          "mirror Auto completes the level and records a hint");

    Act1Level first;
    first.load(*manager.load("1.2"));
    Act1Level second;
    second.load(*manager.load("1.2"));
    check(first.tree().serialize() == second.tree().serialize() &&
              first.optimalSteps() == second.optimalSteps() &&
              first.score().alarmCapacity() == second.score().alarmCapacity(),
          "the same level seed reproduces the same act 1 level");

    return failed;
}
