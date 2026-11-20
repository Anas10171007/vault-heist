#pragma once

#include <string>

#include "model/EventLog.h"
#include "model/Node.h"
#include "model/tree/BinaryTree.h"
#include "util/DynArray.h"

namespace vh {

class BST : public BinaryTree {
public:
    struct OpResult {
        bool ok = false;
        std::string message;
        EventLog events;

        OpResult() = default;
        OpResult(OpResult&&) = default;
        OpResult& operator=(OpResult&&) = default;
        OpResult(const OpResult&) = delete;
        OpResult& operator=(const OpResult&) = delete;
    };

    struct SearchResult {
        BSTNode* found = nullptr;
        EventLog events;

        SearchResult() = default;
        SearchResult(SearchResult&&) = default;
        SearchResult& operator=(SearchResult&&) = default;
        SearchResult(const SearchResult&) = delete;
        SearchResult& operator=(const SearchResult&) = delete;
    };

    struct RangeResult {
        DynArray<int> keys;
        EventLog events;

        RangeResult() = default;
        RangeResult(RangeResult&&) = default;
        RangeResult& operator=(RangeResult&&) = default;
        RangeResult(const RangeResult&) = delete;
        RangeResult& operator=(const RangeResult&) = delete;
    };

    explicit BST(bool recordEvents = true);

    virtual OpResult insert(int key);
    SearchResult search(int key) const;
    virtual OpResult deleteKey(int key);

    BSTNode* findMin() const;
    BSTNode* findMax() const;
    BSTNode* successor(int key) const;
    BSTNode* predecessor(int key) const;
    RangeResult rangeQuery(int lo, int hi) const;
    BSTNode* kthSmallest(int k) const;
    BSTNode* lcaBST(int keyA, int keyB) const;
    void buildChain(int n);

protected:
    TreeNode* createNode(int value) override;

    BSTNode* findNode(int key) const;
    BSTNode* subtreeMin(BSTNode* node) const;
    BSTNode* subtreeMax(BSTNode* node) const;
    void detachNode(BSTNode* node, EventLog& events);
};

}
