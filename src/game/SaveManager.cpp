#include "game/SaveManager.h"
#include <fstream>
#include <sstream>

using namespace std;

namespace vh {

string SaveManager::defaultPath() {
    return "data/save/progress.txt";
}

bool SaveManager::loadProgress(GameSession& session, const string& path) {
    string filePath = path.empty() ? defaultPath() : path;
    const char* prefixes[4] = {"", "../", "../../", "../../../"};
    ifstream in;
    for (int i = 0; i < 4; i++) {
        in.open(string(prefixes[i]) + filePath);
        if (in.good()) break;
        in.clear();
    }
    if (!in.good()) return false;

    string magic;
    in >> magic;
    if (magic != "VHSAVE1") return false;

    session.progress.clear();
    string id;
    int stars = 0;
    while (in >> id >> stars) {
        if (stars < 0) stars = 0;
        if (stars > 3) stars = 3;
        session.recordStars(id, stars);
    }
    return true;
}

bool SaveManager::saveProgress(const GameSession& session, const string& path) {
    string filePath = path.empty() ? defaultPath() : path;
    ofstream out(filePath);
    if (!out.good()) {
        out.open(string("../") + filePath);
    }
    if (!out.good()) {
        out.open(string("../../") + filePath);
    }
    if (!out.good()) return false;

    out << "VHSAVE1\n";
    for (int i = 0; i < session.progress.size(); i++) {
        out << session.progress[i].id << " " << session.progress[i].bestStars << "\n";
    }
    return true;
}

}
