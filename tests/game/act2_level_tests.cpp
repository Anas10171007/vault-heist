#include <iostream>
#include <string>

#include "game/LevelManager.h"
#include "game/LevelPlay.h"
#include "game/LevelPlayFactory.h"
#include "game/act2/Act2Level.h"

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

int runAct2LevelTests() {
    int failed = 0;
    auto check = [&](bool ok, const char* name) {
        cout << (ok ? "PASS " : "FAIL ") << name << endl;
        if (!ok) failed++;
    };

    LevelManager manager;
    bool loaded = manager.loadFile("data/levels/act2.json", 2);
    check(loaded && manager.size() == 5, "act 2 file loads five levels through LevelManager");
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
    check(allValid, "every act 2 level creates a playable logic object");
    check(allOptimal, "every act 2 level computes a positive optimal step count");
    check(allCapacity, "every act 2 level uses the model alarm capacity formula");
    check(allWins, "scripted perfect play wins every act 2 level");
    check(allThreeStars, "scripted perfect play earns three stars on every act 2 level");

    Act2Level search;
    search.load(*manager.load("2.1"));
    int alarmBefore = search.score().alarm();
    int scoreBefore = search.score().score();
    PlayResult wrongSearch = search.apply(clickNode(-77));
    check(wrongSearch.wrong && search.score().alarm() == alarmBefore + 1 &&
              search.score().penalties() == 1 && search.score().score() == scoreBefore - 10,
          "wrong act 2 search click adds one alarm and no flat point penalty");

    Act2Level deletion;
    deletion.load(*manager.load("2.3"));
    int alarmDelete = deletion.score().alarm();
    int scoreDelete = deletion.score().score();
    PlayResult wrongCase = deletion.apply(chooseCase(DeleteCase::LEAF + 99));
    check(wrongCase.wrong && deletion.score().alarm() == alarmDelete + 1 &&
              deletion.score().score() == scoreDelete - 10,
          "wrong delete case is a one-alarm penalty");
    check(playPerfectly(deletion, 100), "delete level still wins after a wrong case");

    Act2Level insert;
    insert.load(*manager.load("2.2"));
    int insertSteps = insert.optimalSteps();
    check(insertSteps > 1 && insert.nextAction().kind == ActionKind::CLICK_NODE,
          "insert optimal steps come from the model search path");
    check(playPerfectly(insert, 100) && insert.score().stars() == 3,
          "scripted insert path wins the insert level");

    Act2Level boss;
    boss.load(*manager.load("2.5"));
    int firstCapacity = boss.score().alarmCapacity();
    bool stageCleared = false;
    for (int guard = 0; guard < 100 && !stageCleared && !boss.isComplete(); guard++) {
        PlayResult result = boss.apply(boss.nextAction());
        if (!result.correct) break;
        if (result.message.find("capacity shrinks") != string::npos) stageCleared = true;
    }
    check(stageCleared && boss.score().alarmCapacity() < firstCapacity,
          "boss chain shrinks alarm capacity after each checkpoint");
    check(playPerfectly(boss, 100) && boss.score().stars() == 3,
          "boss chain still wins with three stars");

    Act2Level hinted;
    hinted.load(*manager.load("2.4"));
    PlayResult hint = hinted.hint();
    check(hint.hasHint && hinted.score().usedHint(), "act 2 hint records its use");
    check(playPerfectly(hinted, 100) && hinted.score().stars() == 2,
          "a hint caps act 2 at two stars");

    Act2Level first;
    first.load(*manager.load("2.2"));
    Act2Level second;
    second.load(*manager.load("2.2"));
    check(first.tree().serialize() == second.tree().serialize() &&
              first.optimalSteps() == second.optimalSteps(),
          "the same level seed reproduces the same act 2 level");

    return failed;
}
