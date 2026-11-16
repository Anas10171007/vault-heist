#pragma once

namespace vh {

struct ScreenId {
    static const int NONE = -1;
    static const int QUIT = -2;
    static const int MENU = 0;
    static const int CAMPAIGN_MAP = 1;
    static const int BRIEFING = 2;
    static const int LEVEL_PLAY = 3;
    static const int RESULT = 4;
    static const int SANDBOX = 5;
    static const int ENDLESS = 6;
    static const int LEADERBOARD = 7;
    static const int SETTINGS = 8;
    static const int COUNT = 9;
};

}
