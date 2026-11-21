#include <iostream>
#include <string>

#include "model/tree/AVLTree.h"
#include "model/tree/BST.h"
#include "model/tree/BinaryTree.h"
#include "model/tree/TreeValidator.h"
#include "util/DynArray.h"
#include "util/Rng.h"

using namespace std;
using namespace vh;

static bool hasCode(const TreeValidationResult& result, int code) {
    for (int i = 0; i < result.problems.size(); i++) {
        if (result.problems[i].code == code) {
            return true;
        }
    }
    return false;
}

static BSTNode* findBSTKey(const BST& tree, int key) {
    TreeNode* current = tree.root();
    while (current != nullptr) {
        BSTNode* node = static_cast<BSTNode*>(current);
        if (key == node->key) {
            return node;
        }
        current = key < node->key ? node->left : node->right;
    }
    return nullptr;
}

static DynArray<int> makeArray(const int* values, int count) {
    DynArray<int> result;
    for (int i = 0; i < count; i++) {
        result.push(values[i]);
    }
    return result;
}

int runTreeValidatorTests() {
    int failed = 0;
    auto check = [&](bool ok, const char* name) {
        cout << (ok ? "PASS " : "FAIL ") << name << endl;
        if (!ok) {
            failed++;
        }
    };

    BST emptyBST;
    AVLTree emptyAVL;
    BinaryTree emptyTree;
    check(TreeValidator::validate(emptyBST).ok() && TreeValidator::validate(emptyAVL).ok() &&
          TreeValidator::validate(emptyTree).ok(),
          "validator accepts empty trees");
    check(TreeValidator::validateTree(nullptr, true, true).ok(),
          "validator accepts a null raw root");

    BST cleanBST;
    int cleanInput[] = {50, 30, 70, 20, 40, 60, 80};
    for (int i = 0; i < 7; i++) {
        cleanBST.insert(cleanInput[i]);
    }
    AVLTree cleanAVL;
    for (int i = 0; i < 7; i++) {
        cleanAVL.insert(cleanInput[i]);
    }
    check(TreeValidator::validate(cleanBST).ok(), "validator accepts a clean BST");
    check(TreeValidator::validate(cleanAVL).ok(), "validator accepts a clean AVL tree");

    string bstBefore = cleanBST.serialize();
    int avlRotations = cleanAVL.rotationCount();
    string avlBefore = cleanAVL.serialize();
    TreeValidationResult readOnlyBST = TreeValidator::validate(cleanBST);
    TreeValidationResult readOnlyAVL = TreeValidator::validate(cleanAVL);
    check(readOnlyBST.ok() && cleanBST.serialize() == bstBefore,
          "validator is read-only on a BST");
    check(readOnlyAVL.ok() && cleanAVL.serialize() == avlBefore &&
          cleanAVL.rotationCount() == avlRotations,
          "validator is read-only on an AVL tree");

    BST orderTree;
    for (int i = 0; i < 7; i++) {
        orderTree.insert(cleanInput[i]);
    }
    BSTNode* low = findBSTKey(orderTree, 20);
    BSTNode* high = findBSTKey(orderTree, 80);
    low->setKey(80);
    high->setKey(20);
    TreeValidationResult orderResult = TreeValidator::validate(orderTree);
    check(!orderResult.ok() && hasCode(orderResult, TreeProblemCode::ORDER),
          "validator catches swapped keys as an ordering problem");
    low->setKey(20);
    high->setKey(80);
    check(TreeValidator::validate(orderTree).ok(), "restoring swapped keys clears the problem");

    BST duplicateKeyTree;
    for (int i = 0; i < 7; i++) {
        duplicateKeyTree.insert(cleanInput[i]);
    }
    findBSTKey(duplicateKeyTree, 20)->setKey(40);
    TreeValidationResult duplicateKeyResult = TreeValidator::validate(duplicateKeyTree);
    check(hasCode(duplicateKeyResult, TreeProblemCode::ORDER),
          "validator catches a duplicated key as an ordering problem");

    AVLTree heightTree;
    for (int i = 0; i < 7; i++) {
        heightTree.insert(cleanInput[i]);
    }
    AVLNode* heightRoot = static_cast<AVLNode*>(heightTree.root());
    int savedHeight = heightRoot->height;
    heightRoot->height = 99;
    TreeValidationResult heightResult = TreeValidator::validate(heightTree);
    check(hasCode(heightResult, TreeProblemCode::HEIGHT),
          "validator catches a wrong stored height");
    heightRoot->height = savedHeight;
    check(TreeValidator::validate(heightTree).ok(), "restoring the height clears the problem");

    AVLNode* sizeRoot = static_cast<AVLNode*>(heightTree.root());
    int savedSize = sizeRoot->size;
    sizeRoot->size = 99;
    TreeValidationResult sizeResult = TreeValidator::validate(heightTree);
    check(hasCode(sizeResult, TreeProblemCode::SIZE),
          "validator catches a wrong stored size");
    sizeRoot->size = savedSize;
    check(TreeValidator::validate(heightTree).ok(), "restoring the size clears the problem");

    BST parentTree;
    parentTree.insert(50);
    parentTree.insert(30);
    TreeNode* parentLeaf = findBSTKey(parentTree, 30);
    TreeNode* realParent = parentLeaf->parent;
    parentLeaf->parent = nullptr;
    TreeValidationResult parentResult = TreeValidator::validate(parentTree);
    check(hasCode(parentResult, TreeProblemCode::PARENT),
          "validator catches a broken parent link");
    parentLeaf->parent = realParent;
    check(TreeValidator::validate(parentTree).ok(), "restoring the parent link clears the problem");

    BST cycleTree;
    cycleTree.insert(50);
    cycleTree.insert(30);
    TreeNode* cycleRoot = cycleTree.root();
    TreeNode* cycleLeaf = cycleRoot->left;
    cycleLeaf->left = cycleRoot;
    TreeValidationResult cycleResult = TreeValidator::validate(cycleTree);
    check(hasCode(cycleResult, TreeProblemCode::CYCLE),
          "validator catches a child cycle without looping");
    cycleLeaf->left = nullptr;
    check(TreeValidator::validate(cycleTree).ok(), "breaking the cycle clears the problem");

    BST sharedTree;
    sharedTree.insert(50);
    sharedTree.insert(30);
    sharedTree.insert(70);
    TreeNode* sharedRoot = sharedTree.root();
    TreeNode* sharedLeft = sharedRoot->left;
    TreeNode* sharedRight = sharedRoot->right;
    sharedRoot->right = sharedLeft;
    TreeValidationResult sharedResult = TreeValidator::validate(sharedTree);
    check(hasCode(sharedResult, TreeProblemCode::SHARED),
          "validator catches a node shared by two parents");
    sharedRoot->right = sharedRight;
    check(TreeValidator::validate(sharedTree).ok(), "restoring the shared child clears the problem");

    AVLTree balanceTree;
    balanceTree.insert(30);
    balanceTree.insert(20);
    balanceTree.insert(40);
    balanceTree.rotateLeft(balanceTree.root()->id);
    TreeValidationResult balanceResult = TreeValidator::validate(balanceTree);
    check(hasCode(balanceResult, TreeProblemCode::BALANCE),
          "validator catches an AVL balance violation");

    BinaryTree generalTree;
    int generalPreorder[] = {1, 3, 2};
    int generalInorder[] = {3, 1, 2};
    BinaryTree::BuildResult generalBuild = generalTree.buildFromPreorderInorder(
        makeArray(generalPreorder, 3), makeArray(generalInorder, 3));
    check(generalBuild.ok && TreeValidator::validate(generalTree).ok(),
          "validator does not impose BST ordering on a general tree");

    BST rawTree;
    rawTree.insert(50);
    rawTree.insert(30);
    rawTree.insert(70);
    findBSTKey(rawTree, 30)->setKey(90);
    TreeValidationResult rawOrdered = TreeValidator::validateTree(rawTree.root(), true, false);
    TreeValidationResult rawUnordered = TreeValidator::validateTree(rawTree.root(), false, false);
    check(hasCode(rawOrdered, TreeProblemCode::ORDER) && rawUnordered.ok(),
          "raw root overload honours the ordering flag");

    BST differentialBST;
    AVLTree differentialAVL;
    Rng rng(20261122u);
    bool bstClean = true;
    bool avlClean = true;
    for (int step = 0; step < 200; step++) {
        int key = rng.range(1, 120);
        bool insertStep = rng.range(0, 1) == 0;
        if (insertStep) {
            differentialBST.insert(key);
            differentialAVL.insert(key);
        } else {
            differentialBST.deleteKey(key);
            differentialAVL.deleteKey(key);
        }
        TreeValidationResult bstResult = TreeValidator::validate(differentialBST);
        TreeValidationResult avlResult = TreeValidator::validate(differentialAVL);
        if (!bstResult.ok()) {
            bstClean = false;
            break;
        }
        if (!avlResult.ok()) {
            avlClean = false;
            break;
        }
    }
    check(bstClean, "clean BST passes the validator after every differential operation");
    check(avlClean, "clean AVL tree passes the validator after every differential operation");

    return failed;
}
