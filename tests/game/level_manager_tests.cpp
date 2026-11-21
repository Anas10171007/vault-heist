#include <iostream>
#include <string>
#include "game/LevelManager.h"

using namespace std;
using namespace vh;

int runLevelManagerTests() {
    int failed = 0;
    auto check = [&](bool ok, const char* name) {
        cout << (ok ? "PASS " : "FAIL ") << name << endl;
        if (!ok) failed++;
    };

    LevelManager manager;
    check(manager.loadAll("data/levels"), "level manager loads all four act files");
    check(manager.size() == 6, "level manager stores every sample level");

    const LevelDef* patrol = manager.load("1.1");
    check(patrol != nullptr && patrol->structure == "binary" && patrol->rules.traversal == "preorder",
          "binary level keeps its structure and traversal rule");

    const LevelDef* search = manager.load("2.1");
    check(search != nullptr && search->data.values.size() == 7 && search->rules.target == 60,
          "BST level keeps values and target");

    const LevelDef* heap = manager.load("4.1");
    check(heap != nullptr && heap->task == "heap_build" && heap->data.values[4] == 95,
          "Act 4 sample is found by id");
    const LevelDef* repair = manager.load("4.2");
    check(repair != nullptr && repair->task == "heap_repair" && repair->data.values[0] == 20,
          "Act 4 repair level keeps its disturbed root");
    check(manager.load("9.9") == nullptr, "unknown level id returns null");

    LevelManager futureFields;
    check(futureFields.loadFile("tests/data/level_unknown_field.json", 4) && futureFields.size() == 1,
          "unknown JSON fields are ignored");

    LevelManager missingTitle;
    bool missingRejected = !missingTitle.loadFile("tests/data/level_missing_title.json", 4);
    check(missingRejected && missingTitle.error().find("level 4.8 missing field title") != string::npos,
          "missing field error names the level and field");

    LevelManager duplicate;
    bool firstLoad = duplicate.loadFile("data/levels/act4.json", 4);
    bool secondLoad = duplicate.loadFile("data/levels/act4.json", 4);
    check(firstLoad && !secondLoad && duplicate.error().find("duplicate level id") != string::npos,
          "duplicate level ids are rejected");

    return failed;
}
