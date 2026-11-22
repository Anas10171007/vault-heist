#pragma once
#include <string>
#include "util/DynArray.h"

using namespace std;

namespace vh {

struct LevelProgress {
    string id;
    int bestStars;
};

struct GameSession {
    string selectedLevel = "1.1";
    string nextLevelId;
    string resultMessage = "No mission completed yet.";
    string sortedArrayText;
    int score = 0;
    int stars = 0;
    int steps = 0;
    int optimalSteps = 0;
    int penalties = 0;
    float animationSpeed = 1.f;
    bool showHelp = true;
    bool finalVaultUnlocked = false;
    // Result → Replay: LevelScreen reopens the last Act 4 timeline without reset.
    bool hasLevelReplay = false;
    bool reviewReplay = false;
    DynArray<LevelProgress> progress;

    int bestStarsFor(const string& levelId) const {
        for (int i = 0; i < progress.size(); i++) {
            if (progress[i].id == levelId) return progress[i].bestStars;
        }
        return 0;
    }

    void recordStars(const string& levelId, int earned) {
        for (int i = 0; i < progress.size(); i++) {
            if (progress[i].id == levelId) {
                if (earned > progress[i].bestStars) progress[i].bestStars = earned;
                return;
            }
        }
        LevelProgress entry;
        entry.id = levelId;
        entry.bestStars = earned;
        progress.push(entry);
    }
};

}
