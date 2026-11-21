#pragma once

namespace vh {

struct PenaltyKind {
    static constexpr int WRONG_TRAVERSAL = 0;
    static constexpr int WRONG_ROTATION = 1;
    static constexpr int HINT = 2;
    static constexpr int INVALID_HEAP_ACTION = 3;
};

class ScoreSystem {
public:
    ScoreSystem(int optimalSteps = 1);

    void addStep();
    void addPenalty(int kind);
    void addBonus(int points);
    void finish();
    void reset(int newOptimalSteps);
    // Race / special levels: shrink capacity after reset (keeps current alarm).
    void setAlarmCapacity(int newCapacity);

    bool isFailed() const;
    bool isFinished() const;
    int stars() const;
    int score() const;
    int steps() const;
    int optimalSteps() const;
    int alarm() const;
    int alarmCapacity() const;
    int penalties() const;
    bool usedHint() const;

private:
    int optimalStepCount;
    int capacity;
    int stepCount;
    int alarmSteps;
    int penaltyCount;
    int pointPenalty;
    int bonusPoints;
    bool hintUsed;
    bool finished;

    void addAlarm(int amount);
    bool canChange() const;
};

}
