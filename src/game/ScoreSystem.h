#pragma once

namespace vh {

// Additive memento used by LevelPlay snapshots.
struct ScoreSnapshot {
    int optimalSteps = 1;
    int capacity = 1;
    int stepCount = 0;
    int alarmSteps = 0;
    int penaltyCount = 0;
    int pointPenalty = 0;
    int bonusPoints = 0;
    bool hintUsed = false;
    bool finished = false;
};

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

    bool isFailed() const;
    bool isFinished() const;
    int stars() const;
    int score() const;
    int steps() const;
    int alarm() const;
    int alarmCapacity() const;
    int penalties() const;
    bool usedHint() const;

    ScoreSnapshot capture() const;
    void restoreState(const ScoreSnapshot& snapshot);

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
