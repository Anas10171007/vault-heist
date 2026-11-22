#pragma once

#include <string>

#include "model/tree/BST.h"

namespace vh {

class AVLTree : public BST {
public:
    explicit AVLTree(bool recordEvents = true);

    OpResult rotateLeft(int nodeId);
    OpResult rotateRight(int nodeId);
    int balanceFactor(const TreeNode* node) const;
    BSTNode* kth(int k) const;
    int rank(int key) const;
    int countInRange(int lo, int hi) const;
    bool isBalanced() const;
    void recomputeStats();
    int rotationCount() const;
    int lastRotations() const;

    BinaryTree::BuildResult deserialize(const std::string& text);
    BinaryTree::BuildResult buildFromPreorderInorder(const DynArray<int>& preorder,
                                                     const DynArray<int>& inorder);
    BinaryTree::BuildResult buildFromPostorderInorder(const DynArray<int>& postorder,
                                                      const DynArray<int>& inorder);

protected:
    TreeNode* createNode(int value) override;
    void captureExtras(const TreeNode* node, int& first, int& second) const override;
    void applyExtras(TreeNode* node, int first, int second) override;
    void afterInsert(BSTNode* inserted, EventLog& events) override;
    void afterRemoval(BSTNode* parentOfRemoved, EventLog& events) override;

private:
    struct Frame {
        AVLNode* node = nullptr;
        int stage = 0;
        int leftHeight = 0;
        int rightHeight = 0;
        int leftSize = 0;
        int rightSize = 0;
    };

    int rotationsTotal = 0;
    int rotationsLast = 0;

    static int heightOf(const AVLNode* node);
    static int sizeOf(const AVLNode* node);
    static int maxOf(int a, int b);
    static AVLNode* asAVL(TreeNode* node);
    static const AVLNode* asAVL(const TreeNode* node);

    void updateStats(AVLNode* node, EventLog& events);
    void rebalanceFrom(AVLNode* start, EventLog& events);
    AVLNode* rotateLeftAt(AVLNode* pivot, int rotationType, EventLog& events);
    AVLNode* rotateRightAt(AVLNode* pivot, int rotationType, EventLog& events);
    int countLess(int key) const;
    bool postorderStats(AVLNode* root, bool writeStats) const;
    void finishLoad(BinaryTree::BuildResult& result);
};

}
