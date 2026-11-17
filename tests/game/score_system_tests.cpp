#include <iostream>
#include "game/ScoreSystem.h"

using namespace std;
using namespace vh;

int runScoreSystemTests() {
    int failed = 0;
    auto check = [&](bool ok, const char* name) {
        cout << (ok ? "PASS " : "FAIL ") << name << endl;
        if (!ok) failed++;
    };

    ScoreSystem capacity(5);
    check(capacity.alarmCapacity() == 10, "alarm capacity rounds ceil 1.5 times optimal then adds two");

    ScoreSystem threeStars(10);
    for (int i = 0; i < 10; i++) threeStars.addStep();
    threeStars.finish();
    check(threeStars.stars() == 3, "optimal steps with no penalties earns three stars");

    ScoreSystem twoStars(10);
    for (int i = 0; i < 12; i++) twoStars.addStep();
    twoStars.addPenalty(PenaltyKind::HINT);
    twoStars.finish();
    check(twoStars.stars() == 2 && twoStars.usedHint(), "one hint caps an otherwise good run at two stars");

    ScoreSystem oneStar(10);
    for (int i = 0; i < 13; i++) oneStar.addStep();
    oneStar.finish();
    check(oneStar.stars() == 1, "completion within capacity earns one star");

    ScoreSystem failedRun(2);
    for (int i = 0; i < failedRun.alarmCapacity(); i++) failedRun.addStep();
    failedRun.finish();
    check(failedRun.isFailed() && failedRun.stars() == 0 && !failedRun.isFinished(),
          "full alarm fails the level and earns zero stars");

    ScoreSystem rotation(10);
    rotation.addPenalty(PenaltyKind::WRONG_ROTATION);
    check(rotation.alarm() == 2 && rotation.penalties() == 1 && rotation.score() == 930,
          "wrong rotation adds two alarm and a fifty point penalty");

    ScoreSystem heapPenalty(10);
    heapPenalty.addPenalty(PenaltyKind::INVALID_HEAP_ACTION);
    check(heapPenalty.alarm() == 1 && heapPenalty.score() == 990,
          "invalid heap action adds one alarm step");

    ScoreSystem resetTest(4);
    resetTest.addStep();
    resetTest.addBonus(25);
    resetTest.reset(8);
    check(resetTest.steps() == 0 && resetTest.alarm() == 0 && resetTest.score() == 1000 &&
          resetTest.alarmCapacity() == 14, "reset clears a run and accepts a new optimum");

    ScoreSystem floorTest(100);
    for (int i = 0; i < 100; i++) floorTest.addStep();
    floorTest.addPenalty(PenaltyKind::WRONG_ROTATION);
    check(floorTest.score() == 0, "score never goes below zero");

    return failed;
}
