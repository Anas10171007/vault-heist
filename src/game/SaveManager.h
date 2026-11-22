#pragma once
#include <string>
#include "game/GameSession.h"

using namespace std;

namespace vh {

class SaveManager {
public:
    static string defaultPath();
    static bool loadProgress(GameSession& session, const string& path = "");
    static bool saveProgress(const GameSession& session, const string& path = "");
};

}
