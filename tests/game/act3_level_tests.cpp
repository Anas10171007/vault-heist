#include <iostream>
#include <string>

#include "game/LevelManager.h"
#include "game/LevelPlay.h"
#include "game/LevelPlayFactory.h"
#include "game/act3/Act3Level.h"

using namespace std;
using namespace vh;

static bool capacityMatches(const LevelPlay& level) {
    int optimal = level.optimalSteps();
    return level.score().alarmCapacity() == (optimal * 3 + 1) / 2 + 2;
}

static void recordCases(const LevelDef& def, const PlayerAction& next, const PlayResult& result,
                        int counts[4]) {
    if (def.task == "avl_rotate_double" || def.task == "avl_auto_predict") {
        if (next.kind == ActionKind::CHOOSE_CASE && next.choice >= 0 && next.choice < 4) {
            counts[next.choice]++;
        }
    }
    for (int i = 0; i < result.events.size(); i++) {
        if (result.events.at(i).type == EventType::ROTATE) {
            int type = result.events.at(i).extra;
            if (type >= 0 && type < 4) counts[type]++;
        }
    }
}

int runAct3LevelTests() {
    int failed = 0;
    auto check = [&](bool ok, const char* name) {
        cout << (ok ? "PASS " : "FAIL ") << name << endl;
        if (!ok) failed++;
    };

    LevelManager manager;
    bool loaded = manager.loadFile("data/levels/act3.json", 3);
    check(loaded && manager.size() == 5, "act 3 file loads five levels through LevelManager");
    if (!loaded) return failed;

    int counts[4] = {0, 0, 0, 0};
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
        int guard = 0;
        while (!play->isComplete() && !play->isFailed() && guard < 100) {
            PlayerAction next = play->nextAction();
            PlayResult result = play->apply(next);
            if (!result.correct) break;
            recordCases(def, next, result, counts);
            guard++;
        }
        if (!play->isComplete() || play->isFailed()) allWins = false;
        if (play->score().stars() != 3) allThreeStars = false;
        delete play;
    }
    check(allValid, "every act 3 level creates a playable logic object");
    check(allOptimal, "every act 3 level computes a positive optimal step count");
    check(allCapacity, "every act 3 level uses the model alarm capacity formula");
    check(allWins, "scripted perfect play wins every act 3 level");
    check(allThreeStars, "scripted perfect play earns three stars on every act 3 level");
    check(counts[RotationType::LL] >= 2 && counts[RotationType::RR] >= 2 &&
              counts[RotationType::LR] >= 2 && counts[RotationType::RL] >= 2,
          "act 3 play produces each rotation type at least twice");

    Act3Level single;
    single.load(*manager.load("3.2"));
    int alarmBefore = single.score().alarm();
    int scoreBefore = single.score().score();
    PlayResult wrongPivot = single.apply(clickNode(-123));
    check(wrongPivot.wrong && single.score().alarm() == alarmBefore + 2 &&
              single.score().penalties() == 1 && single.score().score() == scoreBefore - 70,
          "wrong rotation adds two alarm and a fifty point penalty");

    Act3Level doubled;
    doubled.load(*manager.load("3.3"));
    int alarmDouble = doubled.score().alarm();
    PlayResult wrongCase = doubled.apply(chooseCase(-5));
    check(wrongCase.wrong && doubled.score().alarm() == alarmDouble + 2,
          "wrong double-rotation case charges the rotation penalty");

    Act3Level predicted;
    predicted.load(*manager.load("3.4"));
    PlayerAction prediction = predicted.nextAction();
    prediction.choice = prediction.choice == 0 ? 1 : 0;
    int alarmPredict = predicted.score().alarm();
    PlayResult wrongPrediction = predicted.apply(prediction);
    check(wrongPrediction.wrong && predicted.score().alarm() == alarmPredict + 2,
          "wrong rotation prediction charges the rotation penalty");

    Act3Level badged;
    badged.load(*manager.load("3.1"));
    int alarmBadge = badged.score().alarm();
    PlayResult wrongBadge = badged.apply(clickNode(-9));
    check(wrongBadge.wrong && badged.score().alarm() == alarmBadge + 1 &&
              badged.score().score() == 1000 - 10,
          "wrong badge click charges only one alarm");

    Act3Level hinted;
    hinted.load(*manager.load("3.1"));
    PlayResult hint = hinted.hint();
    check(hint.hasHint && hinted.score().usedHint(), "act 3 hint records its use");
    int guard = 0;
    while (!hinted.isComplete() && !hinted.isFailed() && guard < 100) {
        if (!hinted.apply(hinted.nextAction()).correct) break;
        guard++;
    }
    check(hinted.isComplete() && hinted.score().stars() == 2,
          "a hint caps act 3 at two stars");

    Act3Level first;
    first.load(*manager.load("3.4"));
    Act3Level second;
    second.load(*manager.load("3.4"));
    bool reproducible = first.tree().serialize() == second.tree().serialize() &&
                        first.optimalSteps() == second.optimalSteps();
    for (int guard2 = 0; guard2 < 5 && reproducible; guard2++) {
        PlayerAction one = first.nextAction();
        PlayerAction two = second.nextAction();
        reproducible = one.kind == two.kind && one.choice == two.choice &&
                       one.id == two.id && one.value == two.value;
        first.apply(one);
        second.apply(two);
    }
    check(reproducible, "the same level seed reproduces the same act 3 level");

    return failed;
}
