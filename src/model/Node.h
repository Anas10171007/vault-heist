#pragma once
#include "model/NodeId.h"
namespace vh {
struct TreeNode {
    int id;
    int value;
    TreeNode* left = nullptr;
    TreeNode* right = nullptr;
    TreeNode* parent = nullptr;
    explicit TreeNode(int v) : id(nextNodeId()), value(v) {}
    TreeNode(int nodeId, int v) : id(nodeId), value(v) {
        reserveNodeId(nodeId);
    }
    virtual ~TreeNode() = default;
    TreeNode(const TreeNode&) = delete;
    TreeNode& operator=(const TreeNode&) = delete;
};
struct BSTNode : TreeNode {
    int key;
    int treasure;
    explicit BSTNode(int k, int t = 0) : TreeNode(k), key(k), treasure(t) {}
    BSTNode(int nodeId, int k, int t) : TreeNode(nodeId, k), key(k), treasure(t) {}
    void setKey(int k) {
        key = k;
        value = k;
    }
};
struct AVLNode : BSTNode {
    int height = 1;
    int size = 1;
    explicit AVLNode(int k, int t = 0) : BSTNode(k, t) {}
    AVLNode(int nodeId, int k, int t) : BSTNode(nodeId, k, t) {}
};
}