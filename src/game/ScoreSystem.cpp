#include "game/ScoreSystem.h"

namespace vh {

ScoreSystem::ScoreSystem(int optimalSteps) {
    reset(optimalSteps);
}

void ScoreSystem::reset(int newOptimalSteps) {
    optimalStepCount = newOptimalSteps < 1 ? 1 : newOptimalSteps;
    capacity = (optimalStepCount * 3 + 1) / 2 + 2;
    stepCount = 0;
    alarmSteps = 0;
    penaltyCount = 0;
    pointPenalty = 0;
    bonusPoints = 0;
    hintUsed = false;
    finished = false;
}

bool ScoreSystem::canChange() const {
    return !finished && !isFailed();
}

void ScoreSystem::addAlarm(int amount) {
    alarmSteps += amount;
    if (alarmSteps > capacity) alarmSteps = capacity;
}

void ScoreSystem::addStep() {
    if (!canChange()) return;
    stepCount++;
    addAlarm(1);
}

void ScoreSystem::addPenalty(int kind) {
    if (!canChange()) return;
    penaltyCount++;

    if (kind == PenaltyKind::WRONG_ROTATION) {
        addAlarm(2);
        pointPenalty += 50;
    } else if (kind == PenaltyKind::HINT) {
        hintUsed = true;
    } else if (kind == PenaltyKind::WRONG_TRAVERSAL || kind == PenaltyKind::INVALID_HEAP_ACTION) {
        addAlarm(1);
    }
}

void ScoreSystem::addBonus(int points) {
    if (!canChange() || points < 0) return;
    bonusPoints += points;
}

void ScoreSystem::finish() {
    if (!isFailed()) finished = true;
}

bool ScoreSystem::isFailed() const {
    return alarmSteps >= capacity;
}

bool ScoreSystem::isFinished() const {
    return finished;
}

int ScoreSystem::stars() const {
    if (!finished || isFailed()) return 0;

    int result = 1;
    if (stepCount <= optimalStepCount && penaltyCount == 0) {
        result = 3;
    } else if (stepCount <= optimalStepCount * 1.25 && penaltyCount <= 1) {
        result = 2;
    }

    if (hintUsed && result > 2) result = 2;
    return result;
}

int ScoreSystem::score() const {
    int result = 1000 - 10 * alarmSteps - pointPenalty + bonusPoints;
    return result < 0 ? 0 : result;
}

int ScoreSystem::steps() const {
    return stepCount;
}

int ScoreSystem::optimalSteps() const {
    return optimalStepCount;
}

int ScoreSystem::alarm() const {
    return alarmSteps;
}

int ScoreSystem::alarmCapacity() const {
    return capacity;
}

int ScoreSystem::penalties() const {
    return penaltyCount;
}

bool ScoreSystem::usedHint() const {
    return hintUsed;
}

}
