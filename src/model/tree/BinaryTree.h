#pragma once

#include <string>

#include "model/EventLog.h"
#include "model/Node.h"
#include "model/NodeId.h"
#include "util/DynArray.h"

namespace vh {

// A structural snapshot of the live tree, including node ids and the
// type-specific extras (height and size for AVL nodes).
struct TreeSnapshot {
    DynArray<int> ids;
    DynArray<int> values;
    DynArray<int> leftIds;
    DynArray<int> rightIds;
    DynArray<int> extraA;
    DynArray<int> extraB;
    int rootId = 0;
};

class BinaryTree {
public:
    struct TraversalResult {
        DynArray<int> ids;
        EventLog events;

        TraversalResult() = default;
        TraversalResult(TraversalResult&&) = default;
        TraversalResult& operator=(TraversalResult&&) = default;
        TraversalResult(const TraversalResult&) = delete;
        TraversalResult& operator=(const TraversalResult&) = delete;
    };

    struct BuildResult {
        bool ok = false;
        std::string error;
        EventLog events;

        BuildResult() = default;
        BuildResult(BuildResult&&) = default;
        BuildResult& operator=(BuildResult&&) = default;
        BuildResult(const BuildResult&) = delete;
        BuildResult& operator=(const BuildResult&) = delete;
    };

    explicit BinaryTree(bool recordEvents = true);
    virtual ~BinaryTree();

    BinaryTree(const BinaryTree&) = delete;
    BinaryTree& operator=(const BinaryTree&) = delete;
    BinaryTree(BinaryTree&& other) noexcept;
    BinaryTree& operator=(BinaryTree&& other) noexcept;

    void clear();
    bool empty() const;
    int size() const;
    TreeNode* root() const;
    void setRecordEvents(bool enabled);
    bool isRecording() const;

    TraversalResult preorder() const;
    TraversalResult inorder() const;
    TraversalResult postorder() const;
    TraversalResult levelOrder() const;
    TraversalResult preorderIterative() const;
    TraversalResult inorderIterative() const;
    TraversalResult postorderIterative() const;
    TraversalResult levelOrderIterative() const;

    BuildResult buildFromPreorderInorder(const DynArray<int>& preorder, const DynArray<int>& inorder);
    BuildResult buildFromPostorderInorder(const DynArray<int>& postorder, const DynArray<int>& inorder);

    int height() const;
    int height(const TreeNode* node) const;
    int diameter() const;
    TreeNode* lca(int firstId, int secondId) const;
    DynArray<int> pathTo(int nodeId) const;
    EventLog mirror();
    bool isSameTree(const BinaryTree& other) const;
    bool isSymmetric() const;

    std::string serialize() const;
    BuildResult deserialize(const std::string& text);

    TreeNode* findById(int nodeId) const;
    TreeNode* findByValue(int value) const;
    bool swapChildren(int nodeId);

    TreeSnapshot captureTree() const;
    void restoreTree(const TreeSnapshot& snapshot);

protected:
    virtual void captureExtras(const TreeNode* node, int& first, int& second) const;
    virtual void applyExtras(TreeNode* node, int first, int second);

    TreeNode* rootNode;
    IdSource idSource;
    bool recording;

    virtual TreeNode* createNode(int value);

private:
    static void destroyNodes(TreeNode* node);
    static bool sameNodes(const TreeNode* first, const TreeNode* second);
    static bool mirrorNodes(const TreeNode* first, const TreeNode* second);
    static void skipWhitespace(const std::string& text, int& position);
    static bool parseInteger(const std::string& token, int& value);

    void preorderRec(const TreeNode* node, TraversalResult& result) const;
    void inorderRec(const TreeNode* node, TraversalResult& result) const;
    void postorderRec(const TreeNode* node, TraversalResult& result) const;
    void collectLevel(const TreeNode* node, int level, TraversalResult& result) const;
    void mirrorRec(TreeNode* node);
    int diameterRec(const TreeNode* node, int& best) const;
    int depthOf(const TreeNode* node) const;

    bool validateReconstructionInputs(const DynArray<int>& first, const DynArray<int>& second,
                                      const char* firstName, std::string& error) const;
    TreeNode* buildPreIn(const DynArray<int>& preorder, int preStart, int preEnd,
                         const DynArray<int>& inorder, int inStart, int inEnd,
                         TreeNode* parent, int side, EventLog& events, bool& ok, std::string& error);
    TreeNode* buildPostIn(const DynArray<int>& postorder, int postStart, int postEnd,
                          const DynArray<int>& inorder, int inStart, int inEnd,
                          TreeNode* parent, int side, EventLog& events, bool& ok, std::string& error);
    void serializeRec(const TreeNode* node, std::string& text) const;
    bool parseNode(const std::string& text, int& position, TreeNode* parent, int side,
                   TreeNode*& node, EventLog& events, std::string& error);
};

}
