#include "model/tree/TreeValidator.h"

#include <string>

#include "model/tree/AVLTree.h"
#include "model/tree/BST.h"
#include "util/Stack.h"

namespace vh {

namespace {

class IdSet {
public:
    IdSet() {
        for (int i = 0; i < 8; i++) {
            slots.push(0);
        }
    }

    bool contains(int id) const {
        int mask = slots.size() - 1;
        int index = static_cast<int>(hashOf(id)) & mask;
        while (slots[index] != 0) {
            if (slots[index] == id) {
                return true;
            }
            index = (index + 1) & mask;
        }
        return false;
    }

    void insert(int id) {
        if ((used + 1) * 2 >= slots.size()) {
            grow();
        }
        int mask = slots.size() - 1;
        int index = static_cast<int>(hashOf(id)) & mask;
        while (slots[index] != 0) {
            index = (index + 1) & mask;
        }
        slots[index] = id;
        used++;
    }

private:
    DynArray<int> slots;
    int used = 0;

    static unsigned hashOf(int id) {
        return static_cast<unsigned>(id) * 2654435761u;
    }

    void grow() {
        DynArray<int> old = slots;
        int newSize = slots.size() * 2;
        slots.clear();
        for (int i = 0; i < newSize; i++) {
            slots.push(0);
        }
        used = 0;
        for (int i = 0; i < old.size(); i++) {
            if (old[i] != 0) {
                insert(old[i]);
            }
        }
    }
};

struct VisitFrame {
    const TreeNode* node;
    int stage;
};

struct StatFrame {
    const AVLNode* node;
    int stage;
    int leftHeight;
    int rightHeight;
    int leftSize;
    int rightSize;
};

bool pathContains(const DynArray<const TreeNode*>& path, const TreeNode* node) {
    for (int i = 0; i < path.size(); i++) {
        if (path[i] == node) {
            return true;
        }
    }
    return false;
}

bool hasStructuralProblem(const TreeValidationResult& result) {
    for (int i = 0; i < result.problems.size(); i++) {
        int code = result.problems[i].code;
        if (code == TreeProblemCode::CYCLE || code == TreeProblemCode::SHARED) {
            return true;
        }
    }
    return false;
}

int larger(int a, int b) {
    return a > b ? a : b;
}

}

TreeValidationResult TreeValidator::validate(const BinaryTree& tree) {
    return validateTree(tree);
}

TreeValidationResult TreeValidator::validateTree(const BinaryTree& tree) {
    bool avl = dynamic_cast<const AVLTree*>(&tree) != nullptr;
    bool ordered = avl || dynamic_cast<const BST*>(&tree) != nullptr;
    return validateTree(tree.root(), ordered, avl);
}

TreeValidationResult TreeValidator::validateTree(const TreeNode* root, bool checkOrdering, bool checkAVL) {
    TreeValidationResult result;
    if (root == nullptr) {
        return result;
    }

    IdSet visited;
    Stack<VisitFrame> frames;
    DynArray<const TreeNode*> path;
    VisitFrame first = {root, 0};
    frames.push(first);
    while (!frames.empty()) {
        VisitFrame frame = frames.top();
        if (frame.stage == 0) {
            if (visited.contains(frame.node->id)) {
                bool onPath = pathContains(path, frame.node);
                result.add(onPath ? TreeProblemCode::CYCLE : TreeProblemCode::SHARED, frame.node->id,
                           onPath ? "node is its own ancestor (cycle)"
                                  : "node id is reached twice (shared node or duplicate id)");
                frames.pop();
                continue;
            }
            visited.insert(frame.node->id);
            const TreeNode* expectedParent = path.empty() ? nullptr : path[path.size() - 1];
            if (frame.node->parent != expectedParent) {
                result.add(TreeProblemCode::PARENT, frame.node->id, "node parent link is wrong");
            }
            path.push(frame.node);
            frame.stage = 1;
            frames.top() = frame;
            if (frame.node->left != nullptr) {
                VisitFrame child = {frame.node->left, 0};
                frames.push(child);
            }
        } else if (frame.stage == 1) {
            frame.stage = 2;
            frames.top() = frame;
            if (frame.node->right != nullptr) {
                VisitFrame child = {frame.node->right, 0};
                frames.push(child);
            }
        } else {
            path.pop();
            frames.pop();
        }
    }

    if (hasStructuralProblem(result)) {
        return result;
    }

    if (checkOrdering) {
        Stack<const TreeNode*> pending;
        const TreeNode* current = root;
        bool hasPrevious = false;
        int previousKey = 0;
        while (current != nullptr || !pending.empty()) {
            while (current != nullptr) {
                pending.push(current);
                current = current->left;
            }
            current = pending.pop();
            const BSTNode* node = static_cast<const BSTNode*>(current);
            if (node->key != node->value) {
                result.add(TreeProblemCode::VALUE, node->id, "node key and value differ");
            }
            if (hasPrevious && node->key <= previousKey) {
                result.add(TreeProblemCode::ORDER, node->id, "inorder keys are not strictly ascending");
            }
            previousKey = node->key;
            hasPrevious = true;
            current = node->right;
        }
    }

    if (checkAVL) {
        Stack<StatFrame> stats;
        StatFrame firstStat = {static_cast<const AVLNode*>(root), 0, 0, 0, 0, 0};
        stats.push(firstStat);
        while (!stats.empty()) {
            StatFrame frame = stats.top();
            if (frame.stage == 0) {
                frame.stage = 1;
                stats.top() = frame;
                if (frame.node->left != nullptr) {
                    StatFrame child = {static_cast<const AVLNode*>(frame.node->left), 0, 0, 0, 0, 0};
                    stats.push(child);
                }
            } else if (frame.stage == 1) {
                frame.stage = 2;
                stats.top() = frame;
                if (frame.node->right != nullptr) {
                    StatFrame child = {static_cast<const AVLNode*>(frame.node->right), 0, 0, 0, 0, 0};
                    stats.push(child);
                }
            } else {
                stats.pop();
                int height = 1 + larger(frame.leftHeight, frame.rightHeight);
                int size = 1 + frame.leftSize + frame.rightSize;
                if (frame.node->height != height) {
                    result.add(TreeProblemCode::HEIGHT, frame.node->id,
                               "stored height " + std::to_string(frame.node->height) +
                                   " but recomputed " + std::to_string(height));
                }
                if (frame.node->size != size) {
                    result.add(TreeProblemCode::SIZE, frame.node->id,
                               "stored size " + std::to_string(frame.node->size) +
                                   " but recomputed " + std::to_string(size));
                }
                int balance = frame.leftHeight - frame.rightHeight;
                if (balance > 1 || balance < -1) {
                    result.add(TreeProblemCode::BALANCE, frame.node->id,
                               "balance factor " + std::to_string(balance) + " exceeds one");
                }
                if (!stats.empty()) {
                    StatFrame& parent = stats.top();
                    if (parent.stage == 1) {
                        parent.leftHeight = height;
                        parent.leftSize = size;
                    } else {
                        parent.rightHeight = height;
                        parent.rightSize = size;
                    }
                }
            }
        }
    }

    return result;
}

}
