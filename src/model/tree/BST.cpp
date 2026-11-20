#include "model/tree/BST.h"

#include <string>

#include "util/Stack.h"

namespace vh {

BST::BST(bool recordEvents)
    : BinaryTree(recordEvents) {}

TreeNode* BST::createNode(int value) {
    return new BSTNode(idSource.next(), value);
}

BST::OpResult BST::insert(int key) {
    OpResult result;
    result.events.setRecordEvents(recording);
    BSTNode* parent = nullptr;
    int side = Side::NONE;
    BSTNode* current = static_cast<BSTNode*>(rootNode);
    while (current != nullptr) {
        int comparison = key < current->key ? -1 : (key > current->key ? 1 : 0);
        result.events.append(Event(EventType::COMPARE, current->id, 0, Side::NONE, key, 0, 0, comparison));
        if (comparison == 0) {
            result.ok = false;
            result.message = "duplicate key " + std::to_string(key) + " rejected";
            result.events.append(Event(EventType::DONE));
            return result;
        }
        parent = current;
        if (comparison < 0) {
            side = Side::LEFT;
            current = static_cast<BSTNode*>(current->left);
        } else {
            side = Side::RIGHT;
            current = static_cast<BSTNode*>(current->right);
        }
    }
    BSTNode* node = static_cast<BSTNode*>(createNode(key));
    node->parent = parent;
    if (parent == nullptr) {
        rootNode = node;
    } else if (side == Side::LEFT) {
        parent->left = node;
    } else {
        parent->right = node;
    }
    result.events.append(Event(EventType::INSERT, node->id, parent == nullptr ? 0 : parent->id, side, key));
    result.ok = true;
    result.events.append(Event(EventType::DONE));
    return result;
}

BST::SearchResult BST::search(int key) const {
    SearchResult result;
    result.events.setRecordEvents(recording);
    BSTNode* current = static_cast<BSTNode*>(rootNode);
    while (current != nullptr) {
        int comparison = key < current->key ? -1 : (key > current->key ? 1 : 0);
        result.events.append(Event(EventType::COMPARE, current->id, 0, Side::NONE, key, 0, 0, comparison));
        if (comparison == 0) {
            result.found = current;
            break;
        }
        current = comparison < 0 ? static_cast<BSTNode*>(current->left) : static_cast<BSTNode*>(current->right);
    }
    result.events.append(Event(EventType::DONE));
    return result;
}

BST::OpResult BST::deleteKey(int key) {
    OpResult result;
    result.events.setRecordEvents(recording);
    BSTNode* current = static_cast<BSTNode*>(rootNode);
    BSTNode* target = nullptr;
    while (current != nullptr) {
        int comparison = key < current->key ? -1 : (key > current->key ? 1 : 0);
        result.events.append(Event(EventType::COMPARE, current->id, 0, Side::NONE, key, 0, 0, comparison));
        if (comparison == 0) {
            target = current;
            break;
        }
        current = comparison < 0 ? static_cast<BSTNode*>(current->left) : static_cast<BSTNode*>(current->right);
    }
    if (target == nullptr) {
        result.ok = false;
        result.message = "key " + std::to_string(key) + " not found";
        result.events.append(Event(EventType::DONE));
        return result;
    }
    if (target->left != nullptr && target->right != nullptr) {
        BSTNode* successorNode = subtreeMin(static_cast<BSTNode*>(target->right));
        result.events.append(Event(EventType::HIGHLIGHT_SUBTREE, successorNode->id, 0, Side::NONE, 0, 0, 0,
                                   HighlightMode::RESULT));
        int oldTargetKey = target->key;
        int oldSuccessorKey = successorNode->key;
        target->setKey(oldSuccessorKey);
        successorNode->setKey(oldTargetKey);
        result.events.append(Event(EventType::SWAP_VALUE, target->id, successorNode->id, Side::NONE, 0,
                                   oldTargetKey, oldSuccessorKey));
        detachNode(successorNode, result.events);
    } else {
        detachNode(target, result.events);
    }
    result.ok = true;
    result.events.append(Event(EventType::DONE));
    return result;
}

BSTNode* BST::findMin() const {
    return subtreeMin(static_cast<BSTNode*>(rootNode));
}

BSTNode* BST::findMax() const {
    return subtreeMax(static_cast<BSTNode*>(rootNode));
}

BSTNode* BST::successor(int key) const {
    BSTNode* result = nullptr;
    BSTNode* current = static_cast<BSTNode*>(rootNode);
    while (current != nullptr) {
        if (current->key > key) {
            result = current;
            current = static_cast<BSTNode*>(current->left);
        } else {
            current = static_cast<BSTNode*>(current->right);
        }
    }
    return result;
}

BSTNode* BST::predecessor(int key) const {
    BSTNode* result = nullptr;
    BSTNode* current = static_cast<BSTNode*>(rootNode);
    while (current != nullptr) {
        if (current->key < key) {
            result = current;
            current = static_cast<BSTNode*>(current->right);
        } else {
            current = static_cast<BSTNode*>(current->left);
        }
    }
    return result;
}

BST::RangeResult BST::rangeQuery(int lo, int hi) const {
    RangeResult result;
    result.events.setRecordEvents(recording);
    if (lo <= hi) {
        Stack<BSTNode*> pending;
        BSTNode* current = static_cast<BSTNode*>(rootNode);
        while (current != nullptr || !pending.empty()) {
            if (current != nullptr) {
                if (current->key > lo) {
                    pending.push(current);
                    current = static_cast<BSTNode*>(current->left);
                    continue;
                }
                if (current->left != nullptr) {
                    result.events.append(Event(EventType::HIGHLIGHT_SUBTREE, current->left->id, 0, Side::NONE,
                                               0, 0, 0, HighlightMode::SKIPPED));
                }
            } else {
                current = pending.pop();
            }
            result.events.append(Event(EventType::VISIT, current->id));
            if (current->key >= lo && current->key <= hi) {
                result.keys.push(current->key);
            }
            if (current->key >= hi) {
                if (current->right != nullptr) {
                    result.events.append(Event(EventType::HIGHLIGHT_SUBTREE, current->right->id, 0, Side::NONE,
                                               0, 0, 0, HighlightMode::SKIPPED));
                }
                current = nullptr;
            } else {
                current = static_cast<BSTNode*>(current->right);
            }
        }
    }
    result.events.append(Event(EventType::DONE));
    return result;
}

BSTNode* BST::kthSmallest(int k) const {
    if (k < 1) {
        return nullptr;
    }
    Stack<BSTNode*> pending;
    BSTNode* current = static_cast<BSTNode*>(rootNode);
    int count = 0;
    while (current != nullptr || !pending.empty()) {
        while (current != nullptr) {
            pending.push(current);
            current = static_cast<BSTNode*>(current->left);
        }
        current = pending.pop();
        count++;
        if (count == k) {
            return current;
        }
        current = static_cast<BSTNode*>(current->right);
    }
    return nullptr;
}

BSTNode* BST::lcaBST(int keyA, int keyB) const {
    if (findNode(keyA) == nullptr || findNode(keyB) == nullptr) {
        return nullptr;
    }
    BSTNode* current = static_cast<BSTNode*>(rootNode);
    while (current != nullptr) {
        if (keyA < current->key && keyB < current->key) {
            current = static_cast<BSTNode*>(current->left);
        } else if (keyA > current->key && keyB > current->key) {
            current = static_cast<BSTNode*>(current->right);
        } else {
            return current;
        }
    }
    return nullptr;
}

void BST::buildChain(int n) {
    clear();
    BSTNode* parent = nullptr;
    for (int i = 1; i <= n; i++) {
        BSTNode* node = static_cast<BSTNode*>(createNode(i));
        node->parent = parent;
        if (parent == nullptr) {
            rootNode = node;
        } else {
            parent->right = node;
        }
        parent = node;
    }
}

BSTNode* BST::findNode(int key) const {
    BSTNode* current = static_cast<BSTNode*>(rootNode);
    while (current != nullptr) {
        if (key < current->key) {
            current = static_cast<BSTNode*>(current->left);
        } else if (key > current->key) {
            current = static_cast<BSTNode*>(current->right);
        } else {
            return current;
        }
    }
    return nullptr;
}

BSTNode* BST::subtreeMin(BSTNode* node) const {
    if (node == nullptr) {
        return nullptr;
    }
    while (node->left != nullptr) {
        node = static_cast<BSTNode*>(node->left);
    }
    return node;
}

BSTNode* BST::subtreeMax(BSTNode* node) const {
    if (node == nullptr) {
        return nullptr;
    }
    while (node->right != nullptr) {
        node = static_cast<BSTNode*>(node->right);
    }
    return node;
}

void BST::detachNode(BSTNode* node, EventLog& events) {
    BSTNode* parent = static_cast<BSTNode*>(node->parent);
    BSTNode* child = node->left != nullptr ? static_cast<BSTNode*>(node->left)
                                           : static_cast<BSTNode*>(node->right);
    int side = Side::NONE;
    if (parent != nullptr) {
        side = parent->left == node ? Side::LEFT : Side::RIGHT;
        if (side == Side::LEFT) {
            parent->left = child;
        } else {
            parent->right = child;
        }
    } else {
        rootNode = child;
    }
    if (child != nullptr) {
        child->parent = parent;
    }
    events.append(Event(EventType::UNLINK, node->id, parent == nullptr ? 0 : parent->id, side, node->key,
                        0, 0, child == nullptr ? 0 : child->id));
    delete node;
}

}
