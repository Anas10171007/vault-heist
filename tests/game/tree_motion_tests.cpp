#include <iostream>
#include <string>

#include "game/tree/TreeLayout.h"
#include "game/tree/TreeMotion.h"
#include "model/tree/AVLTree.h"
#include "model/tree/BST.h"

using namespace std;
using namespace vh;

static bool sameLayout(const TreeLayout& first, const TreeLayout& second) {
    if (first.size() != second.size()) return false;
    if (first.bounds().minX != second.bounds().minX ||
        first.bounds().minY != second.bounds().minY ||
        first.bounds().maxX != second.bounds().maxX ||
        first.bounds().maxY != second.bounds().maxY) {
        return false;
    }
    for (int i = 0; i < first.size(); i++) {
        if (first.at(i).nodeId != second.at(i).nodeId || first.at(i).x != second.at(i).x ||
            first.at(i).y != second.at(i).y) {
            return false;
        }
    }
    return true;
}

static bool endedAtFinal(const TreeMotion& motion) {
    return sameLayout(motion.frameAt(motion.eventCount(), 1.f), motion.finalLayout()) &&
           sameLayout(motion.frameAt(motion.eventCount() + 5, 0.5f), motion.finalLayout());
}

static bool startsAtBase(const TreeMotion& motion, const TreeLayout& before) {
    return sameLayout(motion.frameAt(-1, 0.f), before) &&
           sameLayout(motion.frameAt(0, 0.f), before);
}

static void checkSingleRotation(const char* treeText, const char* name, bool rotateRight,
                                int& failed) {
    AVLTree tree;
    tree.deserialize(treeText);
    TreeLayout before = TreeLayout::compute(tree);
    BST::OpResult result = rotateRight ? tree.rotateRight(tree.root()->id)
                                       : tree.rotateLeft(tree.root()->id);
    TreeLayout after = TreeLayout::compute(tree);
    TreeMotion motion = TreeMotion::simple(before, result.events, after);

    bool ok = motion.eventCount() > 0 && startsAtBase(motion, before) && endedAtFinal(motion);
    TreeLayout middle = motion.frameAt(0, 0.5f);
    bool moved = middle.size() == after.size();
    for (int i = 0; i < middle.size(); i++) {
        int id = middle.at(i).nodeId;
        if (before.contains(id) && middle.at(i).x != before.xOf(id) &&
            middle.at(i).x != after.xOf(id)) {
            moved = true;
        }
    }
    ok = ok && moved;
    cout << (ok ? "PASS " : "FAIL ") << name << endl;
    if (!ok) failed++;
}

int runTreeMotionTests() {
    int failed = 0;
    auto check = [&](bool ok, const char* name) {
        cout << (ok ? "PASS " : "FAIL ") << name << endl;
        if (!ok) failed++;
    };

    BST bst;
    int keys[] = {50, 30, 70, 20, 40, 60, 80};
    for (int i = 0; i < 7; i++) bst.insert(keys[i]);
    TreeLayout bstBefore = TreeLayout::compute(bst);
    BST::OpResult inserted = bst.insert(65);
    TreeLayout bstAfter = TreeLayout::compute(bst);
    TreeMotion bstMotion = TreeMotion::simple(bstBefore, inserted.events, bstAfter);
    check(startsAtBase(bstMotion, bstBefore) && endedAtFinal(bstMotion),
          "BST insert motion starts at the old layout and ends at the new layout");
    check(sameLayout(bstMotion.frameAt(bstMotion.eventCount() - 1, 1.f), bstAfter),
          "BST insert replay of every event lands on the final layout");

    TreeLayout deleteBefore = TreeLayout::compute(bst);
    BST::OpResult removed = bst.deleteKey(30);
    TreeLayout deleteAfter = TreeLayout::compute(bst);
    TreeMotion deleteMotion = TreeMotion::simple(deleteBefore, removed.events, deleteAfter);
    check(endedAtFinal(deleteMotion), "BST delete motion ends at the post-delete layout");
    check(!deleteMotion.frameAt(deleteMotion.eventCount(), 1.f).contains(0) &&
              deleteMotion.frameAt(deleteMotion.eventCount(), 1.f).size() == deleteAfter.size(),
          "deleted nodes leave no stale position in the final frame");

    checkSingleRotation("30 20 10 # # # #", "LL rotation motion lands exactly on the new layout",
                        true, failed);
    checkSingleRotation("10 # 20 # 30 # #", "RR rotation motion lands exactly on the new layout",
                        false, failed);

    AVLTree lr;
    lr.deserialize("30 20 # 25 # # #");
    TreeLayout lrBefore = TreeLayout::compute(lr);
    int lrRoot = lr.root()->id;
    int lrPivot = static_cast<AVLNode*>(lr.root()->left)->id;
    BST::OpResult lrFirst = lr.rotateLeft(lrPivot);
    TreeLayout lrMiddle = TreeLayout::compute(lr);
    BST::OpResult lrSecond = lr.rotateRight(lrRoot);
    TreeLayout lrAfter = TreeLayout::compute(lr);
    TreeMotion lrMotion;
    lrMotion.setBase(lrBefore);
    lrMotion.setFinal(lrAfter);
    lrMotion.addEvent(lrFirst.events.at(0), lrMiddle);
    lrMotion.addEvent(lrSecond.events.at(0), lrAfter);
    check(startsAtBase(lrMotion, lrBefore) && endedAtFinal(lrMotion) &&
              sameLayout(lrMotion.frameAt(0, 1.f), lrMiddle) &&
              sameLayout(lrMotion.frameAt(1, 1.f), lrAfter),
          "LR double rotation tween uses both post-rotation layouts");
    check(sameLayout(lrMotion.frameAt(1, 0.f), lrMiddle) &&
              sameLayout(lrMotion.frameAt(1, 1.f), lrAfter) &&
              sameLayout(lrMotion.frameAt(0, 1.f), lrMiddle),
          "LR undo and replay restore exact recorded layouts");

    AVLTree rl;
    rl.deserialize("10 # 20 15 # # #");
    TreeLayout rlBefore = TreeLayout::compute(rl);
    int rlRoot = rl.root()->id;
    int rlPivot = static_cast<AVLNode*>(rl.root()->right)->id;
    BST::OpResult rlFirst = rl.rotateRight(rlPivot);
    TreeLayout rlMiddle = TreeLayout::compute(rl);
    BST::OpResult rlSecond = rl.rotateLeft(rlRoot);
    TreeLayout rlAfter = TreeLayout::compute(rl);
    TreeMotion rlMotion;
    rlMotion.setBase(rlBefore);
    rlMotion.setFinal(rlAfter);
    rlMotion.addEvent(rlFirst.events.at(0), rlMiddle);
    rlMotion.addEvent(rlSecond.events.at(0), rlAfter);
    check(startsAtBase(rlMotion, rlBefore) && endedAtFinal(rlMotion) &&
              sameLayout(rlMotion.frameAt(1, 1.f), rlAfter),
          "RL double rotation tween lands exactly on the new layout");

    TreeMotion empty;
    TreeLayout emptyLayout = TreeLayout::compute(nullptr);
    check(endedAtFinal(empty) && sameLayout(empty.frameAt(-1, 0.f), emptyLayout),
          "an empty motion script still has a stable layout");

    return failed;
}
