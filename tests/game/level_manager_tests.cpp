#include <iostream>
#include <string>
#include "game/LevelDataBuilder.h"
#include "game/LevelManager.h"
#include "model/heap/MaxHeap.h"

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
    check(manager.size() == 20, "level manager stores every authored level");

    const LevelDef* patrol = manager.load("1.1");
    check(patrol != nullptr && patrol->structure == "binary" && patrol->rules.traversal == "preorder",
          "binary level keeps its structure and traversal rule");

    const LevelDef* search = manager.load("2.1");
    check(search != nullptr && search->data.values.size() == 7 && search->rules.target == 60,
          "BST level keeps values and target");

    const LevelDef* heap = manager.load("4.1");
    check(heap != nullptr && heap->task == "heap_build" && heap->data.source == "generate" &&
              heap->data.shape == "sorted" && heap->data.size == 11,
          "Act 4 build level uses seeded ascending generate");
    const LevelDef* repair = manager.load("4.2");
    check(repair != nullptr && repair->task == "heap_repair" &&
              repair->data.shape == "disturbed" && repair->data.disturb == 4,
          "Act 4 repair level uses disturbed generate");
    const LevelDef* sort = manager.load("4.5");
    check(sort != nullptr && sort->task == "heap_sort" && sort->data.shape == "duplicates" &&
              sort->data.size == 12,
          "Act 4 finale uses seeded duplicate-heavy generate");

    DynArray<int> built;
    LevelDataBuilder::buildValues(*heap, built);
    check(built.size() == 11, "generated 4.1 has the requested size");
    MaxHeap ascendingProbe(false);
    ascendingProbe.load(built);
    check(!ascendingProbe.isMaxHeap(), "ascending 4.1 is not already a Max Heap");
    DynArray<int> builtAgain;
    LevelDataBuilder::buildValues(*heap, builtAgain);
    bool sameSeed = built.size() == builtAgain.size();
    for (int i = 0; sameSeed && i < built.size(); i++) {
        if (built[i] != builtAgain[i]) sameSeed = false;
    }
    check(sameSeed, "same seed regenerates the same Act 4 array");

    DynArray<int> validHeap;
    LevelDataBuilder::buildValues(*manager.load("4.3"), validHeap);
    MaxHeap checkHeap(false);
    checkHeap.load(validHeap);
    check(checkHeap.isMaxHeap(), "shape heap starts as a valid Max Heap");

    DynArray<int> broken;
    LevelDataBuilder::buildValues(*repair, broken);
    MaxHeap brokenHeap(false);
    brokenHeap.load(broken);
    check(!brokenHeap.isMaxHeap(), "shape disturbed is not a Max Heap");

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
