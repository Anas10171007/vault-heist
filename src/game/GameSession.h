#pragma once
#include <string>

namespace vh {

struct GameSession {
    std::string selectedLevel = "1.1";
    std::string resultMessage = "No mission completed yet.";
    int score = 0;
    int stars = 0;
    int steps = 0;
    int penalties = 0;
};

}
