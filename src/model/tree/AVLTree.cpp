#include "model/tree/AVLTree.h"

#include <string>

#include "util/Stack.h"

namespace vh {

AVLTree::AVLTree(bool recordEvents)
    : BST(recordEvents) {}

TreeNode* AVLTree::createNode(int value) {
    return new AVLNode(idSource.next(), value);
}

int AVLTree::heightOf(const AVLNode* node) {
    return node == nullptr ? 0 : node->height;
}

int AVLTree::sizeOf(const AVLNode* node) {
    return node == nullptr ? 0 : node->size;
}

int AVLTree::maxOf(int a, int b) {
    return a > b ? a : b;
}

AVLNode* AVLTree::asAVL(TreeNode* node) {
    return static_cast<AVLNode*>(node);
}

const AVLNode* AVLTree::asAVL(const TreeNode* node) {
    return static_cast<const AVLNode*>(node);
}

void AVLTree::updateStats(AVLNode* node, EventLog& events) {
    int oldHeight = node->height;
    int oldSize = node->size;
    node->height = 1 + maxOf(heightOf(asAVL(node->left)), heightOf(asAVL(node->right)));
    node->size = 1 + sizeOf(asAVL(node->left)) + sizeOf(asAVL(node->right));
    if (node->height != oldHeight) {
        events.append(Event(EventType::UPDATE_HEIGHT, node->id, 0, Side::NONE, 0, oldHeight, node->height));
    }
    if (node->size != oldSize) {
        events.append(Event(EventType::UPDATE_SIZE, node->id, 0, Side::NONE, 0, oldSize, node->size));
    }
}

AVLNode* AVLTree::rotateLeftAt(AVLNode* pivot, int rotationType, EventLog& events) {
    AVLNode* rising = asAVL(pivot->right);
    AVLNode* parent = asAVL(pivot->parent);
    AVLNode* middle = asAVL(rising->left);
    events.append(Event(EventType::ROTATE, pivot->id, rising->id, Side::NONE,
                        parent == nullptr ? 0 : parent->id, 0, 0, rotationType));
    pivot->right = middle;
    if (middle != nullptr) {
        middle->parent = pivot;
    }
    rising->left = pivot;
    pivot->parent = rising;
    rising->parent = parent;
    if (parent == nullptr) {
        rootNode = rising;
    } else if (parent->left == pivot) {
        parent->left = rising;
    } else {
        parent->right = rising;
    }
    updateStats(pivot, events);
    updateStats(rising, events);
    rotationsTotal++;
    rotationsLast++;
    return rising;
}

AVLNode* AVLTree::rotateRightAt(AVLNode* pivot, int rotationType, EventLog& events) {
    AVLNode* rising = asAVL(pivot->left);
    AVLNode* parent = asAVL(pivot->parent);
    AVLNode* middle = asAVL(rising->right);
    events.append(Event(EventType::ROTATE, pivot->id, rising->id, Side::NONE,
                        parent == nullptr ? 0 : parent->id, 0, 0, rotationType));
    pivot->left = middle;
    if (middle != nullptr) {
        middle->parent = pivot;
    }
    rising->right = pivot;
    pivot->parent = rising;
    rising->parent = parent;
    if (parent == nullptr) {
        rootNode = rising;
    } else if (parent->left == pivot) {
        parent->left = rising;
    } else {
        parent->right = rising;
    }
    updateStats(pivot, events);
    updateStats(rising, events);
    rotationsTotal++;
    rotationsLast++;
    return rising;
}

void AVLTree::afterInsert(BSTNode* inserted, EventLog& events) {
    rotationsLast = 0;
    rebalanceFrom(asAVL(inserted->parent), events);
}

void AVLTree::afterRemoval(BSTNode* parentOfRemoved, EventLog& events) {
    rotationsLast = 0;
    rebalanceFrom(asAVL(parentOfRemoved), events);
}

void AVLTree::rebalanceFrom(AVLNode* start, EventLog& events) {
    AVLNode* node = start;
    while (node != nullptr) {
        updateStats(node, events);
        int bf = balanceFactor(node);
        AVLNode* next = asAVL(node->parent);
        if (bf > 1) {
            AVLNode* child = asAVL(node->left);
            events.append(Event(EventType::BALANCE_ALERT, node->id, 0, Side::NONE, 0, 0, 0, bf));
            AVLNode* root = nullptr;
            if (balanceFactor(child) >= 0) {
                root = rotateRightAt(node, RotationType::LL, events);
            } else {
                rotateLeftAt(child, RotationType::LR, events);
                root = rotateRightAt(node, RotationType::LR, events);
            }
            next = asAVL(root->parent);
        } else if (bf < -1) {
            AVLNode* child = asAVL(node->right);
            events.append(Event(EventType::BALANCE_ALERT, node->id, 0, Side::NONE, 0, 0, 0, bf));
            AVLNode* root = nullptr;
            if (balanceFactor(child) <= 0) {
                root = rotateLeftAt(node, RotationType::RR, events);
            } else {
                rotateRightAt(child, RotationType::RL, events);
                root = rotateLeftAt(node, RotationType::RL, events);
            }
            next = asAVL(root->parent);
        }
        node = next;
    }
}

int AVLTree::balanceFactor(const TreeNode* node) const {
    if (node == nullptr) {
        return 0;
    }
    const AVLNode* avl = asAVL(node);
    return heightOf(asAVL(avl->left)) - heightOf(asAVL(avl->right));
}

BSTNode* AVLTree::kth(int k) const {
    AVLNode* node = asAVL(rootNode);
    if (k < 1 || k > sizeOf(node)) {
        return nullptr;
    }
    while (node != nullptr) {
        int leftSize = sizeOf(asAVL(node->left));
        if (k == leftSize + 1) {
            return node;
        }
        if (k <= leftSize) {
            node = asAVL(node->left);
        } else {
            k -= leftSize + 1;
            node = asAVL(node->right);
        }
    }
    return nullptr;
}

int AVLTree::countLess(int key) const {
    int count = 0;
    const AVLNode* node = asAVL(rootNode);
    while (node != nullptr) {
        if (node->key < key) {
            count += sizeOf(asAVL(node->left)) + 1;
            node = asAVL(node->right);
        } else {
            node = asAVL(node->left);
        }
    }
    return count;
}

int AVLTree::rank(int key) const {
    int count = 0;
    const AVLNode* node = asAVL(rootNode);
    while (node != nullptr) {
        if (key < node->key) {
            node = asAVL(node->left);
        } else {
            count += sizeOf(asAVL(node->left)) + 1;
            if (key == node->key) {
                return count;
            }
            node = asAVL(node->right);
        }
    }
    return count;
}

int AVLTree::countInRange(int lo, int hi) const {
    if (lo > hi) {
        return 0;
    }
    return rank(hi) - countLess(lo);
}

int AVLTree::rotationCount() const {
    return rotationsTotal;
}

int AVLTree::lastRotations() const {
    return rotationsLast;
}

AVLTree::OpResult AVLTree::rotateLeft(int nodeId) {
    OpResult result;
    result.events.setRecordEvents(recording);
    AVLNode* node = asAVL(findById(nodeId));
    if (node == nullptr) {
        result.message = "node " + std::to_string(nodeId) + " not found";
        result.events.append(Event(EventType::DONE));
        return result;
    }
    if (node->right == nullptr) {
        result.message = "node " + std::to_string(nodeId) + " has no right child";
        result.events.append(Event(EventType::DONE));
        return result;
    }
    rotationsLast = 0;
    rotateLeftAt(node, RotationType::RR, result.events);
    result.ok = true;
    result.events.append(Event(EventType::DONE));
    return result;
}

AVLTree::OpResult AVLTree::rotateRight(int nodeId) {
    OpResult result;
    result.events.setRecordEvents(recording);
    AVLNode* node = asAVL(findById(nodeId));
    if (node == nullptr) {
        result.message = "node " + std::to_string(nodeId) + " not found";
        result.events.append(Event(EventType::DONE));
        return result;
    }
    if (node->left == nullptr) {
        result.message = "node " + std::to_string(nodeId) + " has no left child";
        result.events.append(Event(EventType::DONE));
        return result;
    }
    rotationsLast = 0;
    rotateRightAt(node, RotationType::LL, result.events);
    result.ok = true;
    result.events.append(Event(EventType::DONE));
    return result;
}

bool AVLTree::postorderStats(AVLNode* root, bool writeStats) const {
    Stack<Frame> frames;
    if (root != nullptr) {
        Frame first;
        first.node = root;
        frames.push(first);
    }
    while (!frames.empty()) {
        Frame frame = frames.top();
        if (frame.stage == 0) {
            frame.stage = 1;
            frames.top() = frame;
            if (frame.node->left != nullptr) {
                Frame child;
                child.node = asAVL(frame.node->left);
                frames.push(child);
            }
        } else if (frame.stage == 1) {
            frame.stage = 2;
            frames.top() = frame;
            if (frame.node->right != nullptr) {
                Frame child;
                child.node = asAVL(frame.node->right);
                frames.push(child);
            }
        } else {
            frames.pop();
            int balance = frame.leftHeight - frame.rightHeight;
            if (!writeStats && (balance > 1 || balance < -1)) {
                return false;
            }
            int height = 1 + maxOf(frame.leftHeight, frame.rightHeight);
            int total = 1 + frame.leftSize + frame.rightSize;
            if (writeStats) {
                frame.node->height = height;
                frame.node->size = total;
            }
            if (!frames.empty()) {
                Frame& parent = frames.top();
                if (parent.stage == 1) {
                    parent.leftHeight = height;
                    parent.leftSize = total;
                } else {
                    parent.rightHeight = height;
                    parent.rightSize = total;
                }
            }
        }
    }
    return true;
}

bool AVLTree::isBalanced() const {
    return postorderStats(asAVL(rootNode), false);
}

void AVLTree::recomputeStats() {
    postorderStats(asAVL(rootNode), true);
}

void AVLTree::finishLoad(BinaryTree::BuildResult& result) {
    if (result.ok) {
        recomputeStats();
    }
}

BinaryTree::BuildResult AVLTree::deserialize(const std::string& text) {
    BinaryTree::BuildResult result = BinaryTree::deserialize(text);
    finishLoad(result);
    return result;
}

BinaryTree::BuildResult AVLTree::buildFromPreorderInorder(const DynArray<int>& preorder,
                                                          const DynArray<int>& inorder) {
    BinaryTree::BuildResult result = BinaryTree::buildFromPreorderInorder(preorder, inorder);
    finishLoad(result);
    return result;
}

BinaryTree::BuildResult AVLTree::buildFromPostorderInorder(const DynArray<int>& postorder,
                                                           const DynArray<int>& inorder) {
    BinaryTree::BuildResult result = BinaryTree::buildFromPostorderInorder(postorder, inorder);
    finishLoad(result);
    return result;
}

}
