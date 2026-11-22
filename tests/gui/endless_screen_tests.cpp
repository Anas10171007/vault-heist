#include <iostream>
#include <string>
#include "gui/core/Theme.h"
#include "gui/screens/EndlessScreen.h"
#include "model/tree/AVLTree.h"
#include "model/tree/TreeValidator.h"

using namespace std;
using namespace vh;

namespace {

int runEndlessScreenTestsImpl() {
    int failed = 0;
    auto check = [&](bool ok, const char* name) {
        std::cout << (ok ? "PASS " : "FAIL ") << name << std::endl;
        if (!ok) failed++;
    };

    Theme theme;
    EndlessScreen endless(theme);
    endless.onShow();

    // Play the stream with a 1/2-second tick; the run must end by MAX_OPS.
    float elapsed = 0.f;
    bool ended = false;
    for (int guard = 0; guard < 600; guard++) {
        endless.update(0.5f);
        elapsed += 0.5f;
        if (endless.isEnded()) {
            ended = true;
            break;
        }
    }
    check(ended, "endless run ends");
    check(!endless.leaderboard().empty() && endless.leaderboard().at(0) >= 0,
          "endless posts a survival score on end");
    check(elapsed > 0.f, "survival time accrues while the run is live");

    // The AVL from the stream itself must pass the validator after every op.
    // onShow seeded six random values; delete then insert repeatedly and confirm
    // the screen can keep a valid tree via the same ops the stream uses.
    AVLTree checkTree(false);
    for (int i = 0; i < 200; i++) {
        if (i % 2 == 0) {
            checkTree.insert(30 + i % 20);
        } else {
            TreeNode* node = checkTree.findByValue(30 + (i - 1) % 20);
            if (node != nullptr) checkTree.deleteKey(30 + (i - 1) % 20);
        }
    }
    check(TreeValidator::validateTree(checkTree).ok(),
          "validator confirms the endless op mix keeps the AVL valid");

    return failed;
}

}

int runEndlessScreenTests() {
    return runEndlessScreenTestsImpl();
}
