#pragma once
#include <string>
#include "util/DynArray.h"

using namespace std;

namespace vh {

struct LeaderboardEntry {
    string name;
    int score;
};

class Leaderboard {
public:
    static string defaultPath();

    bool load(const string& path = "");
    bool save(const string& path = "") const;
    void addScore(const string& name, int score);
    int size() const;
    const LeaderboardEntry& at(int index) const;

private:
    DynArray<LeaderboardEntry> entries;
    void sortDescending();
};

}
