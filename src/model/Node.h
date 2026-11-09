#pragma once
namespace vh {
struct TreeNode {
    int id;
    int value;
    TreeNode* left = nullptr;
    TreeNode* right = nullptr;
    TreeNode* parent = nullptr;
    TreeNode(int nodeId, int v) : id(nodeId), value(v) {}
    virtual ~TreeNode() = default;
    TreeNode(const TreeNode&) = delete;
    TreeNode& operator=(const TreeNode&) = delete;
};
struct BSTNode : TreeNode {
    int key;
    int treasure;
    BSTNode(int nodeId, int k, int t = 0) : TreeNode(nodeId, k), key(k), treasure(t) {}
    void setKey(int k) {
        key = k;
        value = k;
    }
};
struct AVLNode : BSTNode {
    int height = 1;
    int size = 1;
    AVLNode(int nodeId, int k, int t = 0) : BSTNode(nodeId, k, t) {}
};
}