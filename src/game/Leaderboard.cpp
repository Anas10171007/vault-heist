#include "game/Leaderboard.h"
#include <fstream>

using namespace std;

namespace vh {

string Leaderboard::defaultPath() {
    return "data/save/leaderboard.txt";
}

void Leaderboard::sortDescending() {
    for (int i = 1; i < entries.size(); i++) {
        LeaderboardEntry key = entries[i];
        int j = i - 1;
        while (j >= 0 && entries[j].score < key.score) {
            entries[j + 1] = entries[j];
            j--;
        }
        entries[j + 1] = key;
    }
}

bool Leaderboard::load(const string& path) {
    string filePath = path.empty() ? defaultPath() : path;
    const char* prefixes[4] = {"", "../", "../../", "../../../"};
    ifstream in;
    for (int i = 0; i < 4; i++) {
        in.open(string(prefixes[i]) + filePath);
        if (in.good()) break;
        in.clear();
    }
    if (!in.good()) {
        entries.clear();
        return false;
    }

    string magic;
    in >> magic;
    if (magic != "VHLEAD1") {
        entries.clear();
        return false;
    }

    entries.clear();
    string name;
    int score = 0;
    while (in >> name >> score) {
        LeaderboardEntry entry;
        entry.name = name;
        entry.score = score;
        entries.push(entry);
    }
    sortDescending();
    return true;
}

bool Leaderboard::save(const string& path) const {
    string filePath = path.empty() ? defaultPath() : path;
    ofstream out(filePath);
    if (!out.good()) out.open(string("../") + filePath);
    if (!out.good()) out.open(string("../../") + filePath);
    if (!out.good()) return false;

    out << "VHLEAD1\n";
    int limit = entries.size() < 10 ? entries.size() : 10;
    for (int i = 0; i < limit; i++) {
        out << entries[i].name << " " << entries[i].score << "\n";
    }
    return true;
}

void Leaderboard::addScore(const string& name, int score) {
    LeaderboardEntry entry;
    entry.name = name.empty() ? "Player" : name;
    entry.score = score;
    entries.push(entry);
    sortDescending();
    while (entries.size() > 10) entries.pop();
}

int Leaderboard::size() const { return entries.size(); }

const LeaderboardEntry& Leaderboard::at(int index) const {
    return entries[index];
}

}
