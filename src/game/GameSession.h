#pragma once
#include <string>

using namespace std;

namespace vh {

struct GameSession {
    string selectedLevel = "1.1";
    string resultMessage = "No mission completed yet.";
    string sortedArrayText;
    int score = 0;
    int stars = 0;
    int steps = 0;
    int penalties = 0;
    float animationSpeed = 1.f;
    bool showHelp = true;
    bool finalVaultUnlocked = false;
};

}
