#include <iostream>
#include <string>

#include "game/tree/TreeLayout.h"
#include "model/tree/AVLTree.h"
#include "model/tree/BST.h"
#include "model/tree/BinaryTree.h"
#include "util/Rng.h"

using namespace std;
using namespace vh;

static bool nearValue(float actual, float expected) {
    float difference = actual - expected;
    return difference <= 0.001f && difference >= -0.001f;
}

static bool noOverlap(const TreeLayout& layout, float spacing) {
    for (int i = 0; i < layout.size(); i++) {
        for (int j = i + 1; j < layout.size(); j++) {
            float gap = layout.at(i).x - layout.at(j).x;
            if (gap < 0.f) {
                gap = -gap;
            }
            if (gap < spacing - 0.001f) {
                return false;
            }
        }
    }
    return true;
}

int runTreeLayoutTests() {
    int failed = 0;
    auto check = [&](bool ok, const char* name) {
        cout << (ok ? "PASS " : "FAIL ") << name << endl;
        if (!ok) {
            failed++;
        }
    };

    const float xSpacing = 40.f;
    const float ySpacing = 70.f;

    BinaryTree emptyTree;
    TreeLayout emptyLayout = TreeLayout::compute(emptyTree, xSpacing, ySpacing);
    check(emptyLayout.empty() && emptyLayout.size() == 0 &&
          nearValue(emptyLayout.bounds().width(), 0.f) &&
          nearValue(emptyLayout.bounds().height(), 0.f),
          "layout of an empty tree is empty with a zero box");
    check(!emptyLayout.contains(1) && nearValue(emptyLayout.xOf(1), 0.f),
          "empty layout has no positions");

    BinaryTree singleTree;
    singleTree.deserialize("42 # #");
    TreeLayout singleLayout = TreeLayout::compute(singleTree, xSpacing, ySpacing);
    check(singleLayout.size() == 1 && singleLayout.contains(singleTree.root()->id),
          "layout of a single node has one position");
    check(nearValue(singleLayout.xOf(singleTree.root()->id), 0.f) &&
          nearValue(singleLayout.yOf(singleTree.root()->id), 0.f),
          "single node sits at the origin");
    check(nearValue(singleLayout.bounds().width(), 0.f) &&
          nearValue(singleLayout.bounds().height(), 0.f),
          "single node has a zero bounding box");

    BST skewed;
    for (int key = 1; key <= 6; key++) {
        skewed.insert(key);
    }
    TreeLayout skewedLayout = TreeLayout::compute(skewed, xSpacing, ySpacing);
    bool skewedPositions = skewedLayout.size() == 6;
    TreeNode* skewedNode = skewed.root();
    int skewedColumn = 0;
    while (skewedNode != nullptr) {
        if (!nearValue(skewedLayout.xOf(skewedNode->id), static_cast<float>(skewedColumn) * xSpacing) ||
            !nearValue(skewedLayout.yOf(skewedNode->id), static_cast<float>(skewedColumn) * ySpacing)) {
            skewedPositions = false;
        }
        skewedNode = skewedNode->right;
        skewedColumn++;
    }
    check(skewedPositions, "skewed tree steps one column and one level per node");
    check(noOverlap(skewedLayout, xSpacing), "skewed tree positions do not overlap");
    check(nearValue(skewedLayout.bounds().width(), 5.f * xSpacing) &&
          nearValue(skewedLayout.bounds().height(), 5.f * ySpacing),
          "skewed tree bounding box spans every node");

    int fullKeys[] = {8, 4, 12, 2, 6, 10, 14, 1, 3, 5, 7, 9, 11, 13, 15};
    BST full;
    for (int i = 0; i < 15; i++) {
        full.insert(fullKeys[i]);
    }
    TreeLayout fullLayout = TreeLayout::compute(full, xSpacing, ySpacing);
    check(fullLayout.size() == 15, "full tree layout has one position per node");
    check(noOverlap(fullLayout, xSpacing), "full tree positions do not overlap");
    check(nearValue(fullLayout.xOf(full.root()->id), 7.f * xSpacing) &&
          nearValue(fullLayout.yOf(full.root()->id), 0.f),
          "full tree root sits at the horizontal centre");
    check(nearValue(fullLayout.bounds().width(), 14.f * xSpacing) &&
          nearValue(fullLayout.bounds().height(), 3.f * ySpacing),
          "full tree bounding box covers four levels");

    Rng rng(20261123u);
    BST randomTree;
    for (int i = 0; i < 200; i++) {
        randomTree.insert(rng.range(1, 100000));
    }
    TreeLayout randomLayout = TreeLayout::compute(randomTree, xSpacing, ySpacing);
    bool allPresent = randomLayout.size() == randomTree.size();
    TreeNode* walker = randomTree.root();
    int visited = 0;
    while (walker != nullptr && visited < randomLayout.size()) {
        if (!randomLayout.contains(walker->id)) {
            allPresent = false;
        }
        walker = walker->right;
        visited++;
    }
    check(allPresent && randomLayout.size() == 200,
          "200-node random layout keys every node id");
    check(noOverlap(randomLayout, xSpacing), "200-node random positions do not overlap");

    BST original;
    BST mirrored;
    for (int i = 0; i < 15; i++) {
        original.insert(fullKeys[i]);
        mirrored.insert(fullKeys[i]);
    }
    mirrored.mirror();
    TreeLayout originalLayout = TreeLayout::compute(original, xSpacing, ySpacing);
    TreeLayout mirroredLayout = TreeLayout::compute(mirrored, xSpacing, ySpacing);
    bool mirrorMatches = originalLayout.size() == mirroredLayout.size();
    TreeNode* originalNode = original.root();
    float mirrorWidth = 14.f * xSpacing;
    while (originalNode != nullptr && mirrorMatches) {
        float expected = mirrorWidth - originalLayout.xOf(originalNode->id);
        if (!nearValue(mirroredLayout.xOf(originalNode->id), expected) ||
            !nearValue(mirroredLayout.yOf(originalNode->id), originalLayout.yOf(originalNode->id))) {
            mirrorMatches = false;
        }
        originalNode = originalNode->left;
    }
    check(mirrorMatches, "mirrored tree gives mirrored x positions");

    AVLTree avl;
    for (int i = 0; i < 15; i++) {
        avl.insert(fullKeys[i]);
    }
    TreeLayout avlLayout = TreeLayout::compute(avl, xSpacing, ySpacing);
    check(avlLayout.size() == avl.size() && noOverlap(avlLayout, xSpacing),
          "AVL tree layout has one non-overlapping position per node");

    TreeLayout rawLayout = TreeLayout::compute(full.root(), xSpacing, ySpacing);
    bool rawMatches = rawLayout.size() == fullLayout.size();
    for (int i = 0; i < fullLayout.size() && rawMatches; i++) {
        if (rawLayout.at(i).nodeId != fullLayout.at(i).nodeId ||
            !nearValue(rawLayout.at(i).x, fullLayout.at(i).x) ||
            !nearValue(rawLayout.at(i).y, fullLayout.at(i).y)) {
            rawMatches = false;
        }
    }
    check(rawMatches, "raw root overload matches the tree overload");

    BST huge;
    for (int key = 1; key <= 10000; key++) {
        huge.insert(key);
    }
    TreeLayout hugeLayout = TreeLayout::compute(huge, xSpacing, ySpacing);
    bool hugeOk = hugeLayout.size() == 10000;
    TreeNode* hugeNode = huge.root();
    int hugeColumn = 0;
    while (hugeNode != nullptr && hugeOk) {
        if (!nearValue(hugeLayout.xOf(hugeNode->id), static_cast<float>(hugeColumn) * xSpacing) ||
            !nearValue(hugeLayout.yOf(hugeNode->id), static_cast<float>(hugeColumn) * ySpacing)) {
            hugeOk = false;
        }
        hugeNode = hugeNode->right;
        hugeColumn++;
    }
    check(hugeOk, "10,000-node skewed layout is iterative and complete");
    check(nearValue(hugeLayout.bounds().height(), 9999.f * ySpacing),
          "10,000-node skewed bounding box spans the full height");

    TreeLayout clamped = TreeLayout::compute(full.root(), 0.f, 0.f);
    check(nearValue(clamped.at(1).x - clamped.at(0).x, 1.f),
          "non-positive spacing is clamped to one");

    return failed;
}
