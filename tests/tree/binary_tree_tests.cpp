#include <iostream>
#include <string>

#include "model/tree/BinaryTree.h"
#include "util/DynArray.h"

using namespace std;
using namespace vh;

static DynArray<int> makeArray(const int* values, int count) {
    DynArray<int> result;
    for (int i = 0; i < count; i++) {
        result.push(values[i]);
    }
    return result;
}

static DynArray<int> valuesOf(const BinaryTree& tree, const DynArray<int>& ids) {
    DynArray<int> result;
    for (int i = 0; i < ids.size(); i++) {
        TreeNode* node = tree.findById(ids[i]);
        result.push(node == nullptr ? 0 : node->value);
    }
    return result;
}

static bool sameArray(const DynArray<int>& first, const DynArray<int>& second) {
    if (first.size() != second.size()) {
        return false;
    }
    for (int i = 0; i < first.size(); i++) {
        if (first[i] != second[i]) {
            return false;
        }
    }
    return true;
}

static int countEvents(const EventLog& log, int type) {
    int count = 0;
    for (int i = 0; i < log.size(); i++) {
        if (log.at(i).type == type) {
            count++;
        }
    }
    return count;
}

static DynArray<int> visitedIds(const EventLog& log) {
    DynArray<int> result;
    for (int i = 0; i < log.size(); i++) {
        if (log.at(i).type == EventType::VISIT) {
            result.push(log.at(i).idA);
        }
    }
    return result;
}

int runBinaryTreeTests() {
    int failed = 0;
    auto check = [&](bool ok, const char* name) {
        cout << (ok ? "PASS " : "FAIL ") << name << endl;
        if (!ok) {
            failed++;
        }
    };

    BinaryTree emptyTree;
    BinaryTree::TraversalResult emptyPre = emptyTree.preorder();
    BinaryTree::TraversalResult emptyIn = emptyTree.inorder();
    BinaryTree::TraversalResult emptyPost = emptyTree.postorder();
    BinaryTree::TraversalResult emptyLevel = emptyTree.levelOrder();
    check(emptyPre.ids.size() == 0 && emptyIn.ids.size() == 0 &&
          emptyPost.ids.size() == 0 && emptyLevel.ids.size() == 0,
          "empty tree traversals are empty");
    check(emptyTree.height() == 0 && emptyTree.diameter() == 0 &&
          emptyTree.size() == 0 && emptyTree.empty(),
          "empty tree measures zero");
    check(emptyTree.serialize() == "#", "empty tree serializes as a null marker");
    check(emptyTree.isSymmetric(), "empty tree is symmetric");
    check(emptyTree.isSameTree(emptyTree), "empty tree equals itself");

    BinaryTree single;
    BinaryTree::BuildResult singleBuilt = single.deserialize("50 # #");
    check(singleBuilt.ok && singleBuilt.error.empty(), "single node deserializes");
    check(single.size() == 1 && single.height() == 1 && single.diameter() == 0,
          "single node measures one");
    check(single.serialize() == "50 # #", "single node round-trips");
    BinaryTree::TraversalResult singlePre = single.preorder();
    BinaryTree::TraversalResult singleIn = single.inorder();
    BinaryTree::TraversalResult singlePost = single.postorder();
    BinaryTree::TraversalResult singleLevel = single.levelOrder();
    check(singlePre.ids.size() == 1 && singleIn.ids.size() == 1 &&
          singlePost.ids.size() == 1 && singleLevel.ids.size() == 1,
          "single node appears once in every traversal");
    check(countEvents(singlePre.events, EventType::VISIT) == 1,
          "traversal emits one VISIT per node");

    const char* smallText = "50 30 20 # # 40 # # 70 60 # # 80 # #";
    BinaryTree small;
    BinaryTree::BuildResult smallBuilt = small.deserialize(smallText);
    check(smallBuilt.ok, "seven-node tree deserializes");
    check(small.size() == 7 && small.height() == 3 && small.diameter() == 4,
          "seven-node tree measures height and diameter");

    int preData[] = {50, 30, 20, 40, 70, 60, 80};
    int inData[] = {20, 30, 40, 50, 60, 70, 80};
    int postData[] = {20, 40, 30, 60, 80, 70, 50};
    int levelData[] = {50, 30, 70, 20, 40, 60, 80};
    DynArray<int> expectedPre = makeArray(preData, 7);
    DynArray<int> expectedIn = makeArray(inData, 7);
    DynArray<int> expectedPost = makeArray(postData, 7);
    DynArray<int> expectedLevel = makeArray(levelData, 7);

    BinaryTree::TraversalResult pre = small.preorder();
    BinaryTree::TraversalResult in = small.inorder();
    BinaryTree::TraversalResult post = small.postorder();
    BinaryTree::TraversalResult level = small.levelOrder();
    check(sameArray(valuesOf(small, pre.ids), expectedPre), "preorder returns the right values");
    check(sameArray(valuesOf(small, in.ids), expectedIn), "inorder returns the right values");
    check(sameArray(valuesOf(small, post.ids), expectedPost), "postorder returns the right values");
    check(sameArray(valuesOf(small, level.ids), expectedLevel), "level order returns the right values");

    BinaryTree::TraversalResult preIt = small.preorderIterative();
    BinaryTree::TraversalResult inIt = small.inorderIterative();
    BinaryTree::TraversalResult postIt = small.postorderIterative();
    BinaryTree::TraversalResult levelIt = small.levelOrderIterative();
    check(sameArray(pre.ids, preIt.ids), "recursive and iterative preorder agree");
    check(sameArray(in.ids, inIt.ids), "recursive and iterative inorder agree");
    check(sameArray(post.ids, postIt.ids), "recursive and iterative postorder agree");
    check(sameArray(level.ids, levelIt.ids), "recursive and iterative level order agree");
    check(sameArray(visitedIds(pre.events), pre.ids), "preorder VISIT events follow the traversal");
    check(sameArray(visitedIds(in.events), in.ids), "inorder VISIT events follow the traversal");
    check(sameArray(visitedIds(post.events), post.ids), "postorder VISIT events follow the traversal");
    check(sameArray(visitedIds(level.events), level.ids), "level order VISIT events follow the traversal");

    bool parentsOk = true;
    for (int i = 0; i < pre.ids.size(); i++) {
        TreeNode* node = small.findById(pre.ids[i]);
        if (node == nullptr) {
            parentsOk = false;
            break;
        }
        if (node->left != nullptr && node->left->parent != node) {
            parentsOk = false;
        }
        if (node->right != nullptr && node->right->parent != node) {
            parentsOk = false;
        }
    }
    check(parentsOk, "parent pointers are consistent after deserialization");

    BinaryTree skewed;
    BinaryTree::BuildResult skewedBuilt = skewed.deserialize("1 # 2 # 3 # 4 # #");
    check(skewedBuilt.ok && skewed.height() == 4 && skewed.diameter() == 3,
          "skewed tree measures height and diameter");
    check(sameArray(skewed.preorder().ids, skewed.preorderIterative().ids),
          "skewed preorder recursive equals iterative");
    check(sameArray(skewed.inorder().ids, skewed.inorderIterative().ids),
          "skewed inorder recursive equals iterative");
    check(sameArray(skewed.postorder().ids, skewed.postorderIterative().ids),
          "skewed postorder recursive equals iterative");
    check(sameArray(skewed.levelOrder().ids, skewed.levelOrderIterative().ids),
          "skewed level order recursive equals iterative");

    BinaryTree rebuiltPre;
    BinaryTree::BuildResult rebuiltPreResult = rebuiltPre.buildFromPreorderInorder(expectedPre, expectedIn);
    check(rebuiltPreResult.ok && rebuiltPre.isSameTree(small),
          "preorder plus inorder rebuilds the tree");
    check(rebuiltPre.serialize() == small.serialize(), "rebuilt tree serializes identically");
    check(countEvents(rebuiltPreResult.events, EventType::INSERT) == 7,
          "rebuild emits one INSERT per node");
    check(rebuiltPreResult.events.size() > 0 &&
          rebuiltPreResult.events.at(rebuiltPreResult.events.size() - 1).type == EventType::DONE,
          "rebuild ends with DONE");

    BinaryTree rebuiltPost;
    BinaryTree::BuildResult rebuiltPostResult = rebuiltPost.buildFromPostorderInorder(expectedPost, expectedIn);
    check(rebuiltPostResult.ok && rebuiltPost.isSameTree(small),
          "postorder plus inorder rebuilds the tree");

    bool rebuiltParentsOk = true;
    BinaryTree::TraversalResult rebuiltIds = rebuiltPre.preorder();
    for (int i = 0; i < rebuiltIds.ids.size(); i++) {
        TreeNode* node = rebuiltPre.findById(rebuiltIds.ids[i]);
        if (node == nullptr) {
            rebuiltParentsOk = false;
            break;
        }
        if (node->left != nullptr && node->left->parent != node) {
            rebuiltParentsOk = false;
        }
        if (node->right != nullptr && node->right->parent != node) {
            rebuiltParentsOk = false;
        }
    }
    check(rebuiltParentsOk, "parent pointers are consistent after reconstruction");

    BinaryTree leftChain;
    BinaryTree::BuildResult leftChainResult = leftChain.deserialize("1 2 3 4 # # # # #");
    check(leftChainResult.ok && leftChain.height() == 4 && leftChain.diameter() == 3,
          "diameter finds a path that does not cross the root");

    int shortPreData[] = {50, 30};
    DynArray<int> shortPre = makeArray(shortPreData, 2);
    int oneInData[] = {50};
    DynArray<int> oneIn = makeArray(oneInData, 1);
    BinaryTree guarded;
    guarded.deserialize(smallText);
    string guardedText = guarded.serialize();
    BinaryTree::BuildResult mismatch = guarded.buildFromPreorderInorder(shortPre, oneIn);
    check(!mismatch.ok && !mismatch.error.empty(), "size mismatch is rejected with a message");
    check(guarded.serialize() == guardedText, "failed rebuild leaves the tree unchanged");

    int duplicatePreData[] = {1, 1};
    int duplicateInData[] = {1, 1};
    DynArray<int> duplicatePre = makeArray(duplicatePreData, 2);
    DynArray<int> duplicateIn = makeArray(duplicateInData, 2);
    BinaryTree::BuildResult duplicate = guarded.buildFromPreorderInorder(duplicatePre, duplicateIn);
    check(!duplicate.ok && duplicate.error.find("duplicate") != string::npos,
          "duplicate reconstruction input is rejected");

    int badPreData[] = {1, 2, 3};
    int badInData[] = {3, 1, 2};
    DynArray<int> badPre = makeArray(badPreData, 3);
    DynArray<int> badIn = makeArray(badInData, 3);
    BinaryTree::BuildResult bad = guarded.buildFromPreorderInorder(badPre, badIn);
    check(!bad.ok, "inconsistent traversals are rejected");

    DynArray<int> noValues;
    BinaryTree::BuildResult emptyBuild = guarded.buildFromPreorderInorder(noValues, noValues);
    check(emptyBuild.ok && guarded.empty(), "empty reconstruction clears the tree");

    auto idOf = [&](int value) {
        BinaryTree::TraversalResult all = small.preorder();
        for (int i = 0; i < all.ids.size(); i++) {
            TreeNode* node = small.findById(all.ids[i]);
            if (node != nullptr && node->value == value) {
                return node->id;
            }
        }
        return 0;
    };

    TreeNode* lcaNode = small.lca(idOf(20), idOf(40));
    check(lcaNode != nullptr && lcaNode->value == 30, "lca of siblings is their parent");
    lcaNode = small.lca(idOf(20), idOf(60));
    check(lcaNode != nullptr && lcaNode->value == 50, "lca across subtrees is the root");
    lcaNode = small.lca(idOf(50), idOf(70));
    check(lcaNode != nullptr && lcaNode->value == 50, "lca of the root and a descendant is the root");
    check(small.lca(9999, idOf(20)) == nullptr, "lca with a missing id returns null");

    DynArray<int> path = small.pathTo(idOf(20));
    int pathData[] = {50, 30, 20};
    check(sameArray(valuesOf(small, path), makeArray(pathData, 3)),
          "pathTo returns the root-to-node path");
    DynArray<int> rootPath = small.pathTo(idOf(50));
    check(rootPath.size() == 1 && valuesOf(small, rootPath)[0] == 50,
          "pathTo the root has one node");
    check(small.pathTo(9999).size() == 0, "pathTo a missing id is empty");

    BinaryTree mirrored;
    mirrored.deserialize(smallText);
    EventLog mirrorEvents = mirrored.mirror();
    check(mirrorEvents.size() > 0 &&
          mirrorEvents.at(mirrorEvents.size() - 1).type == EventType::DONE,
          "mirror returns an event log");
    int mirroredPreData[] = {50, 70, 80, 60, 30, 40, 20};
    int mirroredInData[] = {80, 70, 60, 50, 40, 30, 20};
    check(sameArray(valuesOf(mirrored, mirrored.preorder().ids), makeArray(mirroredPreData, 7)),
          "mirror swaps children recursively");
    check(sameArray(valuesOf(mirrored, mirrored.inorder().ids), makeArray(mirroredInData, 7)),
          "mirror reverses the inorder sequence");
    check(mirrored.height() == 3 && mirrored.diameter() == 4 && mirrored.size() == 7,
          "mirror preserves measurements");
    mirrored.mirror();
    check(mirrored.serialize() == smallText, "mirroring twice restores the tree");

    BinaryTree mirroredIds;
    mirroredIds.deserialize(smallText);
    int rootId = mirroredIds.root()->id;
    int leafId = mirroredIds.preorder().ids[2];
    mirroredIds.mirror();
    check(mirroredIds.findById(rootId) != nullptr && mirroredIds.findById(leafId) != nullptr,
          "node ids survive mirror");

    BinaryTree sameA;
    sameA.deserialize(smallText);
    BinaryTree sameB;
    sameB.deserialize(smallText);
    check(sameA.isSameTree(sameB), "identical trees compare equal");
    BinaryTree differentValue;
    differentValue.deserialize("50 30 20 # # 40 # # 70 60 # # 81 # #");
    check(!sameA.isSameTree(differentValue), "trees with different values differ");
    BinaryTree differentShape;
    differentShape.deserialize("50 30 20 # # # 70 60 # # 80 # #");
    check(!sameA.isSameTree(differentShape), "trees with different shapes differ");

    BinaryTree symmetric;
    symmetric.deserialize("1 2 # # 2 # #");
    check(symmetric.isSymmetric(), "a symmetric tree is detected");
    BinaryTree asymmetric;
    asymmetric.deserialize("1 2 # # 3 # #");
    check(!asymmetric.isSymmetric(), "an asymmetric tree is rejected");
    check(!small.isSymmetric(), "the seven-node sample is not symmetric");

    BinaryTree roundTrip;
    roundTrip.deserialize(smallText);
    string firstText = roundTrip.serialize();
    BinaryTree roundTripAgain;
    BinaryTree::BuildResult again = roundTripAgain.deserialize(firstText);
    check(again.ok && roundTripAgain.serialize() == firstText,
          "serialize and deserialize round-trip");
    check(roundTripAgain.isSameTree(roundTrip), "round-trip preserves structure and values");

    BinaryTree nullMarker;
    nullMarker.deserialize(smallText);
    BinaryTree::BuildResult nullResult = nullMarker.deserialize("#");
    check(nullResult.ok && nullMarker.empty(), "a single null marker clears the tree");

    BinaryTree spaced;
    BinaryTree::BuildResult spacedResult = spaced.deserialize("  50\t30\n#\r# #  ");
    check(spacedResult.ok && spaced.serialize() == "50 30 # # #",
          "whitespace between tokens is accepted");

    BinaryTree overflow;
    BinaryTree::BuildResult overflowResult = overflow.deserialize("2147483648 # #");
    check(!overflowResult.ok, "out-of-range integers are rejected");

    BinaryTree negatives;
    BinaryTree::BuildResult negativeResult = negatives.deserialize("-5 3 # # -2 # #");
    check(negativeResult.ok && negatives.serialize() == "-5 3 # # -2 # #",
          "negative values round-trip");

    BinaryTree quiet;
    quiet.deserialize(smallText);
    quiet.setRecordEvents(false);
    BinaryTree::TraversalResult quietPre = quiet.preorder();
    check(quietPre.events.empty() && quietPre.ids.size() == 7,
          "recording off still returns the traversal");
    quiet.setRecordEvents(true);
    check(quiet.preorder().events.size() > 0, "recording on emits events again");

    BinaryTree quietBuild;
    quietBuild.setRecordEvents(false);
    BinaryTree::BuildResult quietBuildResult = quietBuild.deserialize("1 2 # # 3 # #");
    check(quietBuildResult.ok && quietBuildResult.events.empty() && quietBuild.size() == 3,
          "recording off keeps deserialization working without events");

    BinaryTree quietReconstruct;
    quietReconstruct.setRecordEvents(false);
    BinaryTree::BuildResult quietReconstructResult =
        quietReconstruct.buildFromPreorderInorder(expectedPre, expectedIn);
    check(quietReconstructResult.ok && quietReconstructResult.events.empty() &&
          quietReconstruct.isSameTree(small),
          "recording off keeps reconstruction working without events");

    BinaryTree quietMirror;
    quietMirror.setRecordEvents(false);
    quietMirror.deserialize(smallText);
    EventLog quietMirrorEvents = quietMirror.mirror();
    check(quietMirrorEvents.empty(), "recording off keeps mirror working without events");

    BinaryTree reuse;
    reuse.deserialize("1 # #");
    int oldId = reuse.root()->id;
    reuse.clear();
    reuse.deserialize("2 # #");
    check(reuse.root()->id > oldId, "node ids are not reused after clear");

    BinaryTree guardedDeserialize;
    guardedDeserialize.deserialize(smallText);
    string guardedDeserializeText = guardedDeserialize.serialize();
    BinaryTree::BuildResult incomplete = guardedDeserialize.deserialize("50 30");
    check(!incomplete.ok && !incomplete.error.empty(), "incomplete serialization is rejected");
    check(guardedDeserialize.serialize() == guardedDeserializeText,
          "failed deserialization leaves the tree unchanged");
    BinaryTree::BuildResult badToken = guardedDeserialize.deserialize("50 x #");
    check(!badToken.ok, "invalid token is rejected");
    BinaryTree::BuildResult trailing = guardedDeserialize.deserialize("50 # # 70 # #");
    check(!trailing.ok, "trailing tokens are rejected");
    BinaryTree::BuildResult emptyText = guardedDeserialize.deserialize("");
    check(!emptyText.ok, "empty text is rejected");
    check(guardedDeserialize.serialize() == guardedDeserializeText,
          "every failed deserialization leaves the tree unchanged");

    BinaryTree movable;
    movable.deserialize(smallText);
    BinaryTree moved(static_cast<BinaryTree&&>(movable));
    check(moved.size() == 7 && movable.empty(), "move construction transfers ownership");
    BinaryTree moveTarget;
    moveTarget = static_cast<BinaryTree&&>(moved);
    check(moveTarget.size() == 7 && moved.empty(), "move assignment transfers ownership");

    return failed;
}
