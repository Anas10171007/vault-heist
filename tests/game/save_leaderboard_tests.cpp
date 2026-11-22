#include <iostream>
#include <cstdio>
#include "game/GameSession.h"
#include "game/Leaderboard.h"
#include "game/SaveManager.h"

using namespace std;
using namespace vh;

int runSaveLeaderboardTests() {
    int failed = 0;
    auto check = [&](bool ok, const char* name) {
        cout << (ok ? "PASS " : "FAIL ") << name << endl;
        if (!ok) failed++;
    };

    const string savePath = "data/save/test_progress.txt";
    const string boardPath = "data/save/test_leaderboard.txt";
    remove(savePath.c_str());
    remove(string("../" + savePath).c_str());
    remove(boardPath.c_str());
    remove(string("../" + boardPath).c_str());

    GameSession session;
    session.recordStars("4.1", 2);
    session.recordStars("4.2", 3);
    session.recordStars("4.1", 1);
    check(session.bestStarsFor("4.1") == 2, "recordStars keeps the best star count");
    check(SaveManager::saveProgress(session, savePath), "saveProgress writes VHSAVE1");

    GameSession loaded;
    check(SaveManager::loadProgress(loaded, savePath), "loadProgress reads VHSAVE1");
    check(loaded.bestStarsFor("4.1") == 2 && loaded.bestStarsFor("4.2") == 3,
          "loaded progress matches saved stars");

    Leaderboard board;
    board.addScore("4.1", 120);
    board.addScore("4.3", 90);
    board.addScore("4.2", 200);
    check(board.size() == 3 && board.at(0).score == 200, "leaderboard sorts descending");
    check(board.save(boardPath), "leaderboard save writes VHLEAD1");

    Leaderboard reloaded;
    check(reloaded.load(boardPath), "leaderboard load reads VHLEAD1");
    check(reloaded.size() == 3 && reloaded.at(0).name == "4.2" && reloaded.at(0).score == 200,
          "reloaded leaderboard keeps top score first");

    for (int i = 0; i < 12; i++) board.addScore("P" + to_string(i), i);
    check(board.size() == 10, "leaderboard keeps only top ten");

    remove(savePath.c_str());
    remove(string("../" + savePath).c_str());
    remove(boardPath.c_str());
    remove(string("../" + boardPath).c_str());
    return failed;
}
