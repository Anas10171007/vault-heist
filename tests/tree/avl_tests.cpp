#include <climits>
#include <cmath>
#include <ctime>
#include <iostream>
#include <string>

#include "model/tree/AVLTree.h"
#include "util/DynArray.h"
#include "util/Rng.h"
#include "util/Stack.h"

using namespace std;
using namespace vh;

static int larger(int a, int b) {
    return a > b ? a : b;
}

static DynArray<int> makeArray(const int* values, int count) {
    DynArray<int> result;
    for (int i = 0; i < count; i++) {
        result.push(values[i]);
    }
    return result;
}

static bool sameArray(const DynArray<int>& first, const DynArray<int>& second) {
    if (first.size() != second.size()) {
        return false;
    }
    for (int i = 0; i < first.size(); i++) {
        if (first[i] != second[i]) {
            return false;
        }
    }
    return true;
}

static void mergeSort(DynArray<int>& values) {
    int count = values.size();
    if (count < 2) {
        return;
    }
    DynArray<int> buffer;
    for (int i = 0; i < count; i++) {
        buffer.push(0);
    }
    for (int width = 1; width < count; width *= 2) {
        for (int lo = 0; lo < count; lo += 2 * width) {
            int mid = lo + width;
            if (mid > count) {
                mid = count;
            }
            int hi = lo + 2 * width;
            if (hi > count) {
                hi = count;
            }
            int i = lo;
            int j = mid;
            int k = lo;
            while (i < mid && j < hi) {
                buffer[k++] = values[i] <= values[j] ? values[i++] : values[j++];
            }
            while (i < mid) {
                buffer[k++] = values[i++];
            }
            while (j < hi) {
                buffer[k++] = values[j++];
            }
        }
        for (int i = 0; i < count; i++) {
            values[i] = buffer[i];
        }
    }
}

static bool referenceContains(const DynArray<int>& values, int key) {
    for (int i = 0; i < values.size(); i++) {
        if (values[i] == key) {
            return true;
        }
    }
    return false;
}

static void referenceInsert(DynArray<int>& values, int key) {
    DynArray<int> next;
    bool inserted = false;
    for (int i = 0; i < values.size(); i++) {
        if (!inserted && key < values[i]) {
            next.push(key);
            inserted = true;
        }
        next.push(values[i]);
    }
    if (!inserted) {
        next.push(key);
    }
    values = next;
}

static void referenceRemove(DynArray<int>& values, int key) {
    DynArray<int> next;
    for (int i = 0; i < values.size(); i++) {
        if (values[i] != key) {
            next.push(values[i]);
        }
    }
    values = next;
}

static int referenceRank(const DynArray<int>& values, int key) {
    int count = 0;
    for (int i = 0; i < values.size(); i++) {
        if (values[i] <= key) {
            count++;
        }
    }
    return count;
}

static AVLNode* findKey(const AVLTree& tree, int key) {
    TreeNode* current = tree.root();
    while (current != nullptr) {
        AVLNode* node = static_cast<AVLNode*>(current);
        if (key == node->key) {
            return node;
        }
        current = key < node->key ? node->left : node->right;
    }
    return nullptr;
}

static void collectKeys(const AVLTree& tree, DynArray<int>& keys) {
    Stack<TreeNode*> pending;
    TreeNode* current = tree.root();
    while (current != nullptr || !pending.empty()) {
        while (current != nullptr) {
            pending.push(current);
            current = current->left;
        }
        current = pending.pop();
        keys.push(static_cast<AVLNode*>(current)->key);
        current = current->right;
    }
}

static void collectIds(const AVLTree& tree, DynArray<int>& ids) {
    Stack<TreeNode*> pending;
    TreeNode* current = tree.root();
    while (current != nullptr || !pending.empty()) {
        while (current != nullptr) {
            pending.push(current);
            current = current->left;
        }
        current = pending.pop();
        ids.push(current->id);
        current = current->right;
    }
}

// Independent checker. It recomputes height and size with its own iterative
// postorder and never calls AVLTree::isBalanced or recomputeStats.
static bool validateAVL(const AVLTree& tree, string& why, bool checkBalance) {
    Stack<TreeNode*> pending;
    TreeNode* current = tree.root();
    DynArray<AVLNode*> nodes;
    int steps = 0;
    while (current != nullptr || !pending.empty()) {
        while (current != nullptr) {
            pending.push(current);
            current = current->left;
            steps++;
            if (steps > 300000) {
                why = "traversal did not terminate";
                return false;
            }
        }
        current = pending.pop();
        AVLNode* node = static_cast<AVLNode*>(current);
        if (node->key != node->value) {
            why = "key and value differ";
            return false;
        }
        if (!nodes.empty() && nodes[nodes.size() - 1]->key >= node->key) {
            why = "inorder is not strictly ascending";
            return false;
        }
        if (node->left != nullptr) {
            if (node->left->parent != node) {
                why = "left child parent link is wrong";
                return false;
            }
            if (static_cast<AVLNode*>(node->left)->key >= node->key) {
                why = "left child is not smaller";
                return false;
            }
        }
        if (node->right != nullptr) {
            if (node->right->parent != node) {
                why = "right child parent link is wrong";
                return false;
            }
            if (static_cast<AVLNode*>(node->right)->key <= node->key) {
                why = "right child is not larger";
                return false;
            }
        }
        nodes.push(node);
        current = node->right;
    }
    if (tree.root() != nullptr && tree.root()->parent != nullptr) {
        why = "root has a parent";
        return false;
    }
    DynArray<int> ids;
    for (int i = 0; i < nodes.size(); i++) {
        ids.push(nodes[i]->id);
    }
    mergeSort(ids);
    for (int i = 1; i < ids.size(); i++) {
        if (ids[i - 1] == ids[i]) {
            why = "two nodes share an id";
            return false;
        }
    }

    struct CheckFrame {
        AVLNode* node;
        int stage;
        int leftHeight;
        int rightHeight;
        int leftSize;
        int rightSize;
    };
    Stack<CheckFrame> frames;
    if (tree.root() != nullptr) {
        CheckFrame first = {static_cast<AVLNode*>(tree.root()), 0, 0, 0, 0, 0};
        frames.push(first);
    }
    while (!frames.empty()) {
        CheckFrame frame = frames.top();
        if (frame.stage == 0) {
            frame.stage = 1;
            frames.top() = frame;
            if (frame.node->left != nullptr) {
                CheckFrame child = {static_cast<AVLNode*>(frame.node->left), 0, 0, 0, 0, 0};
                frames.push(child);
            }
        } else if (frame.stage == 1) {
            frame.stage = 2;
            frames.top() = frame;
            if (frame.node->right != nullptr) {
                CheckFrame child = {static_cast<AVLNode*>(frame.node->right), 0, 0, 0, 0, 0};
                frames.push(child);
            }
        } else {
            frames.pop();
            int height = 1 + larger(frame.leftHeight, frame.rightHeight);
            int size = 1 + frame.leftSize + frame.rightSize;
            if (frame.node->height != height) {
                why = "stored height is wrong";
                return false;
            }
            if (frame.node->size != size) {
                why = "stored size is wrong";
                return false;
            }
            int balance = frame.leftHeight - frame.rightHeight;
            if (checkBalance && (balance > 1 || balance < -1)) {
                why = "balance factor exceeds one";
                return false;
            }
            if (!frames.empty()) {
                CheckFrame& parent = frames.top();
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
    return true;
}

static int eventCount(const EventLog& log, int type) {
    int count = 0;
    for (int i = 0; i < log.size(); i++) {
        if (log.at(i).type == type) {
            count++;
        }
    }
    return count;
}

static int firstEvent(const EventLog& log, int type) {
    for (int i = 0; i < log.size(); i++) {
        if (log.at(i).type == type) {
            return i;
        }
    }
    return -1;
}

int runAVLTests() {
    int failed = 0;
    auto check = [&](bool ok, const char* name) {
        cout << (ok ? "PASS " : "FAIL ") << name << endl;
        if (!ok) {
            failed++;
        }
    };
    string why;

    AVLTree empty;
    check(empty.root() == nullptr && empty.size() == 0 && empty.height() == 0,
          "empty AVL tree measures zero");
    check(empty.isBalanced() && empty.rotationCount() == 0 && empty.lastRotations() == 0,
          "empty AVL tree is balanced with zero rotations");
    check(empty.kth(1) == nullptr && empty.rank(5) == 0 && empty.countInRange(1, 9) == 0,
          "empty AVL tree answers kth, rank and countInRange");

    AVLTree ll;
    ll.insert(30);
    ll.insert(20);
    AVLTree::OpResult llInsert = ll.insert(10);
    check(ll.serialize() == "20 10 # # 30 # #", "LL insert gives the expected shape");
    check(ll.root() != nullptr && static_cast<AVLNode*>(ll.root())->key == 20 &&
          ll.root()->parent == nullptr && ll.root()->left->parent == ll.root() &&
          ll.root()->right->parent == ll.root(),
          "LL insert fixes the root and parent links");
    check(llInsert.events.size() == 13, "LL insert emits the full event sequence");
    check(llInsert.events.at(0).type == EventType::COMPARE &&
          llInsert.events.at(1).type == EventType::COMPARE &&
          llInsert.events.at(2).type == EventType::INSERT,
          "LL insert compares before INSERT");
    check(llInsert.events.at(3).type == EventType::UPDATE_HEIGHT &&
          llInsert.events.at(4).type == EventType::UPDATE_SIZE,
          "LL insert updates height before size");
    check(llInsert.events.at(7).type == EventType::BALANCE_ALERT &&
          llInsert.events.at(7).idA == findKey(ll, 30)->id && llInsert.events.at(7).extra == 2,
          "LL insert alerts on the unbalanced node");
    check(llInsert.events.at(8).type == EventType::ROTATE &&
          llInsert.events.at(8).idA == findKey(ll, 30)->id && llInsert.events.at(8).idB == findKey(ll, 20)->id &&
          llInsert.events.at(8).key == 0 && llInsert.events.at(8).extra == RotationType::LL,
          "LL ROTATE carries pivot, rising node, parent and case");
    check(llInsert.events.at(9).type == EventType::UPDATE_HEIGHT &&
          llInsert.events.at(9).oldValue == 3 && llInsert.events.at(9).newValue == 1,
          "rotation updates the lowered node first");
    check(llInsert.events.at(12).type == EventType::DONE, "LL insert ends with DONE");
    check(ll.rotationCount() == 1 && ll.lastRotations() == 1, "LL insert counts one rotation");
    check(validateAVL(ll, why, true), "LL insert passes the checker");

    AVLTree rr;
    rr.insert(10);
    rr.insert(20);
    AVLTree::OpResult rrInsert = rr.insert(30);
    check(rr.serialize() == "20 10 # # 30 # #", "RR insert gives the expected shape");
    check(eventCount(rrInsert.events, EventType::ROTATE) == 1 &&
          rrInsert.events.at(firstEvent(rrInsert.events, EventType::ROTATE)).extra == RotationType::RR,
          "RR insert emits one RR rotation");
    check(rrInsert.events.at(firstEvent(rrInsert.events, EventType::ROTATE)).idA == findKey(rr, 10)->id,
          "RR pivot is the unbalanced root");
    check(rr.rotationCount() == 1 && rr.lastRotations() == 1, "RR insert counts one rotation");
    check(validateAVL(rr, why, true), "RR insert passes the checker");

    AVLTree lr;
    lr.insert(30);
    lr.insert(10);
    AVLTree::OpResult lrInsert = lr.insert(20);
    check(lr.serialize() == "20 10 # # 30 # #", "LR insert gives the expected shape");
    int lrFirst = firstEvent(lrInsert.events, EventType::ROTATE);
    check(lrFirst >= 0 && lrInsert.events.at(lrFirst).idA == findKey(lr, 10)->id &&
          lrInsert.events.at(lrFirst).idB == findKey(lr, 20)->id &&
          lrInsert.events.at(lrFirst).key == findKey(lr, 30)->id &&
          lrInsert.events.at(lrFirst).extra == RotationType::LR,
          "LR first rotation pivots the child");
    check(lrFirst >= 0 && lrFirst + 1 < lrInsert.events.size() &&
          lrInsert.events.at(lrFirst + 1).type == EventType::UPDATE_HEIGHT,
          "LR first rotation updates height before the second rotation");
    int lrSecond = -1;
    for (int i = lrFirst + 1; i < lrInsert.events.size(); i++) {
        if (lrInsert.events.at(i).type == EventType::ROTATE) {
            lrSecond = i;
            break;
        }
    }
    check(lrSecond >= 0 && lrInsert.events.at(lrSecond).idA == findKey(lr, 30)->id &&
          lrInsert.events.at(lrSecond).extra == RotationType::LR,
          "LR second rotation pivots the unbalanced node");
    check(lr.rotationCount() == 2 && lr.lastRotations() == 2, "LR insert counts two rotations");
    check(validateAVL(lr, why, true), "LR insert passes the checker");

    AVLTree rl;
    rl.insert(10);
    rl.insert(30);
    AVLTree::OpResult rlInsert = rl.insert(20);
    check(rl.serialize() == "20 10 # # 30 # #", "RL insert gives the expected shape");
    int rlFirst = firstEvent(rlInsert.events, EventType::ROTATE);
    check(rlFirst >= 0 && rlInsert.events.at(rlFirst).idA == findKey(rl, 30)->id &&
          rlInsert.events.at(rlFirst).extra == RotationType::RL,
          "RL first rotation pivots the child");
    int rlSecond = -1;
    for (int i = rlFirst + 1; i < rlInsert.events.size(); i++) {
        if (rlInsert.events.at(i).type == EventType::ROTATE) {
            rlSecond = i;
            break;
        }
    }
    check(rlSecond >= 0 && rlInsert.events.at(rlSecond).idA == findKey(rl, 10)->id &&
          rlInsert.events.at(rlSecond).extra == RotationType::RL,
          "RL second rotation pivots the unbalanced node");
    check(rl.rotationCount() == 2 && rl.lastRotations() == 2, "RL insert counts two rotations");
    check(validateAVL(rl, why, true), "RL insert passes the checker");

    auto insertAll = [](AVLTree& tree, const int* values, int count) {
        for (int i = 0; i < count; i++) {
            tree.insert(values[i]);
        }
    };

    int nonRootLL[] = {100, 50, 150, 30, 70, 130, 10, 5};
    AVLTree deepLL;
    insertAll(deepLL, nonRootLL, 7);
    AVLTree::OpResult deepLLResult = deepLL.insert(5);
    check(deepLL.root() != nullptr && static_cast<AVLNode*>(deepLL.root())->key == 100,
          "non-root LL keeps the root");
    check(findKey(deepLL, 10) != nullptr && findKey(deepLL, 10)->parent == findKey(deepLL, 50) &&
          findKey(deepLL, 50)->left == findKey(deepLL, 10),
          "non-root LL relinks the rotated subtree to its parent");
    check(eventCount(deepLLResult.events, EventType::ROTATE) == 1 &&
          deepLLResult.events.at(firstEvent(deepLLResult.events, EventType::ROTATE)).idA == findKey(deepLL, 30)->id,
          "non-root LL pivots the inner node");
    check(validateAVL(deepLL, why, true), "non-root LL passes the checker");

    int nonRootRR[] = {100, 150, 50, 170, 130, 70, 190, 200};
    AVLTree deepRR;
    insertAll(deepRR, nonRootRR, 7);
    AVLTree::OpResult deepRRResult = deepRR.insert(200);
    check(deepRR.root() != nullptr && static_cast<AVLNode*>(deepRR.root())->key == 100,
          "non-root RR keeps the root");
    check(findKey(deepRR, 190) != nullptr && findKey(deepRR, 190)->parent == findKey(deepRR, 150) &&
          findKey(deepRR, 150)->right == findKey(deepRR, 190),
          "non-root RR relinks the rotated subtree to its parent");
    check(eventCount(deepRRResult.events, EventType::ROTATE) == 1 &&
          deepRRResult.events.at(firstEvent(deepRRResult.events, EventType::ROTATE)).idA == findKey(deepRR, 170)->id,
          "non-root RR pivots the inner node");
    check(validateAVL(deepRR, why, true), "non-root RR passes the checker");

    int nonRootLR[] = {100, 50, 150, 30, 70, 130, 20, 40, 45};
    AVLTree deepLR;
    insertAll(deepLR, nonRootLR, 8);
    AVLTree::OpResult deepLRResult = deepLR.insert(45);
    check(deepLR.root() != nullptr && static_cast<AVLNode*>(deepLR.root())->key == 100,
          "non-root LR keeps the root");
    check(findKey(deepLR, 40) != nullptr && findKey(deepLR, 40)->parent == findKey(deepLR, 100) &&
          findKey(deepLR, 100)->left == findKey(deepLR, 40),
          "non-root LR relinks the rotated subtree to its parent");
    check(eventCount(deepLRResult.events, EventType::ROTATE) == 2 &&
          deepLRResult.events.at(firstEvent(deepLRResult.events, EventType::ROTATE)).idA == findKey(deepLR, 30)->id,
          "non-root LR pivots the child first");
    check(validateAVL(deepLR, why, true), "non-root LR passes the checker");

    int nonRootRL[] = {100, 50, 150, 30, 70, 130, 80, 60, 55};
    AVLTree deepRL;
    insertAll(deepRL, nonRootRL, 8);
    AVLTree::OpResult deepRLResult = deepRL.insert(55);
    check(deepRL.root() != nullptr && static_cast<AVLNode*>(deepRL.root())->key == 100,
          "non-root RL keeps the root");
    check(findKey(deepRL, 60) != nullptr && findKey(deepRL, 60)->parent == findKey(deepRL, 100) &&
          findKey(deepRL, 100)->left == findKey(deepRL, 60),
          "non-root RL relinks the rotated subtree to its parent");
    check(eventCount(deepRLResult.events, EventType::ROTATE) == 2 &&
          deepRLResult.events.at(firstEvent(deepRLResult.events, EventType::ROTATE)).idA == findKey(deepRL, 70)->id,
          "non-root RL pivots the child first");
    check(validateAVL(deepRL, why, true), "non-root RL passes the checker");

    int rootLL[] = {100, 50, 150, 30, 70, 10};
    AVLTree rootLLTree;
    insertAll(rootLLTree, rootLL, 5);
    AVLTree::OpResult rootLLResult = rootLLTree.insert(10);
    check(rootLLTree.root() != nullptr && static_cast<AVLNode*>(rootLLTree.root())->key == 50 &&
          rootLLTree.root()->parent == nullptr,
          "root LL replaces the root and clears its parent");
    check(eventCount(rootLLResult.events, EventType::ROTATE) == 1 &&
          rootLLResult.events.at(firstEvent(rootLLResult.events, EventType::ROTATE)).idA == findKey(rootLLTree, 100)->id &&
          rootLLResult.events.at(firstEvent(rootLLResult.events, EventType::ROTATE)).key == 0,
          "root LL ROTATE records parent 0");
    check(validateAVL(rootLLTree, why, true), "root LL passes the checker");

    int rootRR[] = {100, 150, 50, 170, 130, 190};
    AVLTree rootRRTree;
    insertAll(rootRRTree, rootRR, 5);
    AVLTree::OpResult rootRRResult = rootRRTree.insert(190);
    check(rootRRTree.root() != nullptr && static_cast<AVLNode*>(rootRRTree.root())->key == 150 &&
          rootRRTree.root()->parent == nullptr,
          "root RR replaces the root and clears its parent");
    check(eventCount(rootRRResult.events, EventType::ROTATE) == 1 &&
          rootRRResult.events.at(firstEvent(rootRRResult.events, EventType::ROTATE)).idA == findKey(rootRRTree, 100)->id,
          "root RR pivots the old root");
    check(validateAVL(rootRRTree, why, true), "root RR passes the checker");

    int rootLR[] = {100, 50, 150, 30, 70, 80};
    AVLTree rootLRTree;
    insertAll(rootLRTree, rootLR, 5);
    AVLTree::OpResult rootLRResult = rootLRTree.insert(80);
    check(rootLRTree.root() != nullptr && static_cast<AVLNode*>(rootLRTree.root())->key == 70 &&
          rootLRTree.root()->parent == nullptr,
          "root LR replaces the root and clears its parent");
    check(eventCount(rootLRResult.events, EventType::ROTATE) == 2,
          "root LR performs two primitive rotations");
    check(validateAVL(rootLRTree, why, true), "root LR passes the checker");

    int rootRL[] = {100, 50, 150, 130, 170, 120};
    AVLTree rootRLTree;
    insertAll(rootRLTree, rootRL, 5);
    AVLTree::OpResult rootRLResult = rootRLTree.insert(120);
    check(rootRLTree.root() != nullptr && static_cast<AVLNode*>(rootRLTree.root())->key == 130 &&
          rootRLTree.root()->parent == nullptr,
          "root RL replaces the root and clears its parent");
    check(eventCount(rootRLResult.events, EventType::ROTATE) == 2,
          "root RL performs two primitive rotations");
    check(validateAVL(rootRLTree, why, true), "root RL passes the checker");

    AVLTree ascending;
    bool ascendingOk = true;
    for (int i = 1; i <= 1000; i++) {
        AVLTree::OpResult result = ascending.insert(i);
        ascendingOk = ascendingOk && result.ok && validateAVL(ascending, why, true);
    }
    check(ascendingOk, "ascending inserts stay valid after every insert");
    check(ascending.height() == 10 && ascending.size() == 1000,
          "1000 ascending inserts give height 10");
    DynArray<int> ascendingKeys;
    collectKeys(ascending, ascendingKeys);
    DynArray<int> oneToThousand;
    for (int i = 1; i <= 1000; i++) {
        oneToThousand.push(i);
    }
    check(sameArray(ascendingKeys, oneToThousand), "ascending inserts keep the sorted inorder");

    AVLTree descending;
    bool descendingOk = true;
    for (int i = 1000; i >= 1; i--) {
        AVLTree::OpResult result = descending.insert(i);
        descendingOk = descendingOk && result.ok && validateAVL(descending, why, true);
    }
    check(descendingOk, "descending inserts stay valid after every insert");
    check(descending.height() == 10 && descending.size() == 1000,
          "1000 descending inserts give height 10");

    DynArray<int> randomKeys;
    for (int i = 1; i <= 1000; i++) {
        randomKeys.push(i);
    }
    Rng rng(20261121u);
    rng.shuffle(randomKeys);
    AVLTree randomTree;
    bool randomOk = true;
    for (int i = 0; i < randomKeys.size(); i++) {
        AVLTree::OpResult result = randomTree.insert(randomKeys[i]);
        randomOk = randomOk && result.ok && validateAVL(randomTree, why, true);
    }
    double randomBound = 1.44 * log2(1002.0);
    check(randomOk && randomTree.height() <= randomBound && randomTree.size() == 1000,
          "1000 random inserts stay balanced within the AVL bound");

    AVLTree deletes;
    int deleteInput[] = {50, 30, 70, 20, 40, 60, 80, 10, 25, 35, 45, 55, 65, 75, 85};
    insertAll(deletes, deleteInput, 15);
    check(validateAVL(deletes, why, true), "delete fixture starts balanced");

    int deleteRootId = deletes.root()->id;
    AVLTree::OpResult leafDelete = deletes.deleteKey(10);
    check(leafDelete.ok && findKey(deletes, 10) == nullptr && deletes.size() == 14,
          "AVL leaf delete removes the key");
    check(deletes.lastRotations() == 0, "leaf delete needs no rotation");
    check(validateAVL(deletes, why, true), "AVL leaf delete passes the checker");

    AVLTree::OpResult oneChildDelete = deletes.deleteKey(20);
    check(oneChildDelete.ok && findKey(deletes, 20) == nullptr && findKey(deletes, 25) != nullptr,
          "AVL one-child delete bypasses the child");
    check(validateAVL(deletes, why, true), "AVL one-child delete passes the checker");

    AVLTree::OpResult twoChildDelete = deletes.deleteKey(30);
    check(twoChildDelete.ok && findKey(deletes, 30) == nullptr && findKey(deletes, 35) != nullptr,
          "AVL two-child delete removes the target key");
    check(validateAVL(deletes, why, true), "AVL two-child delete passes the checker");

    AVLTree::OpResult rootDelete = deletes.deleteKey(50);
    check(rootDelete.ok && deletes.root() != nullptr && deletes.root()->id == deleteRootId &&
          static_cast<AVLNode*>(deletes.root())->key == 55,
          "AVL root delete keeps the root id and takes the successor key");
    check(validateAVL(deletes, why, true), "AVL root delete passes the checker");

    string beforeMissing = deletes.serialize();
    int sizeBeforeMissing = deletes.size();
    int rotationsBeforeMissing = deletes.rotationCount();
    AVLTree::OpResult missingDelete = deletes.deleteKey(999);
    check(!missingDelete.ok && missingDelete.message == "key 999 not found",
          "AVL missing delete reports the exact message");
    check(deletes.serialize() == beforeMissing && deletes.size() == sizeBeforeMissing &&
          deletes.rotationCount() == rotationsBeforeMissing &&
          eventCount(missingDelete.events, EventType::ROTATE) == 0,
          "AVL missing delete leaves the tree and counters untouched");
    check(missingDelete.events.at(missingDelete.events.size() - 1).type == EventType::DONE,
          "AVL missing delete ends with DONE");

    AVLTree duplicates;
    insertAll(duplicates, deleteInput, 15);
    string beforeDuplicate = duplicates.serialize();
    int duplicateRotations = duplicates.rotationCount();
    AVLTree::OpResult duplicateInsert = duplicates.insert(40);
    check(!duplicateInsert.ok && duplicateInsert.message == "duplicate key 40 rejected",
          "AVL duplicate insert reports the exact message");
    check(duplicates.serialize() == beforeDuplicate && duplicates.rotationCount() == duplicateRotations,
          "AVL duplicate insert leaves the tree untouched");
    check(eventCount(duplicateInsert.events, EventType::INSERT) == 0 &&
          eventCount(duplicateInsert.events, EventType::ROTATE) == 0 &&
          eventCount(duplicateInsert.events, EventType::UPDATE_HEIGHT) == 0 &&
          eventCount(duplicateInsert.events, EventType::UPDATE_SIZE) == 0 &&
          eventCount(duplicateInsert.events, EventType::BALANCE_ALERT) == 0 &&
          duplicateInsert.events.at(duplicateInsert.events.size() - 1).type == EventType::DONE,
          "AVL duplicate insert emits only comparisons and DONE");

    AVLTree deleteLL;
    BinaryTree::BuildResult loadLL = deleteLL.deserialize("50 30 20 10 # # # 40 # # 70 # 80 # #");
    check(loadLL.ok && validateAVL(deleteLL, why, true), "delete LL fixture loads balanced");
    int deleteLLPivot = findKey(deleteLL, 50)->id;
    AVLTree::OpResult deleteLLResult = deleteLL.deleteKey(80);
    check(deleteLLResult.ok && deleteLL.lastRotations() == 1,
          "delete LL performs a single rotation");
    check(eventCount(deleteLLResult.events, EventType::ROTATE) == 1 &&
          deleteLLResult.events.at(firstEvent(deleteLLResult.events, EventType::ROTATE)).idA == deleteLLPivot &&
          deleteLLResult.events.at(firstEvent(deleteLLResult.events, EventType::ROTATE)).extra == RotationType::LL,
          "delete LL emits one LL rotation at the unbalanced node");
    check(deleteLL.root() != nullptr && static_cast<AVLNode*>(deleteLL.root())->key == 30 &&
          validateAVL(deleteLL, why, true),
          "delete LL leaves a valid tree rooted at 30");

    AVLTree deleteRR;
    BinaryTree::BuildResult loadRR = deleteRR.deserialize("50 30 20 # # # 70 60 # # 80 # 90 # #");
    check(loadRR.ok && validateAVL(deleteRR, why, true), "delete RR fixture loads balanced");
    int deleteRRPivot = findKey(deleteRR, 50)->id;
    AVLTree::OpResult deleteRRResult = deleteRR.deleteKey(20);
    check(deleteRRResult.ok && deleteRR.lastRotations() == 1,
          "delete RR performs a single rotation");
    check(eventCount(deleteRRResult.events, EventType::ROTATE) == 1 &&
          deleteRRResult.events.at(firstEvent(deleteRRResult.events, EventType::ROTATE)).idA == deleteRRPivot &&
          deleteRRResult.events.at(firstEvent(deleteRRResult.events, EventType::ROTATE)).extra == RotationType::RR,
          "delete RR emits one RR rotation at the unbalanced node");
    check(deleteRR.root() != nullptr && static_cast<AVLNode*>(deleteRR.root())->key == 70 &&
          validateAVL(deleteRR, why, true),
          "delete RR leaves a valid tree rooted at 70");

    AVLTree deleteLR;
    BinaryTree::BuildResult loadLR = deleteLR.deserialize("50 30 20 # # 40 # 45 # # 70 # 80 # #");
    check(loadLR.ok && validateAVL(deleteLR, why, true), "delete LR fixture loads balanced");
    int deleteLRChild = findKey(deleteLR, 30)->id;
    int deleteLRPivot = findKey(deleteLR, 50)->id;
    AVLTree::OpResult deleteLRResult = deleteLR.deleteKey(80);
    check(deleteLRResult.ok && deleteLR.lastRotations() == 2,
          "delete LR performs two primitive rotations");
    int deleteLRFirst = firstEvent(deleteLRResult.events, EventType::ROTATE);
    int deleteLRSecond = -1;
    for (int i = deleteLRFirst + 1; i < deleteLRResult.events.size(); i++) {
        if (deleteLRResult.events.at(i).type == EventType::ROTATE) {
            deleteLRSecond = i;
            break;
        }
    }
    check(deleteLRFirst >= 0 && deleteLRResult.events.at(deleteLRFirst).idA == deleteLRChild &&
          deleteLRResult.events.at(deleteLRFirst).extra == RotationType::LR &&
          deleteLRSecond >= 0 && deleteLRResult.events.at(deleteLRSecond).idA == deleteLRPivot &&
          deleteLRResult.events.at(deleteLRSecond).extra == RotationType::LR,
          "delete LR labels both primitives LR in order");
    check(deleteLR.root() != nullptr && static_cast<AVLNode*>(deleteLR.root())->key == 40 &&
          validateAVL(deleteLR, why, true),
          "delete LR leaves a valid tree rooted at 40");

    AVLTree deleteRL;
    BinaryTree::BuildResult loadRL = deleteRL.deserialize("50 30 20 # # # 70 60 55 # # # 80 # #");
    check(loadRL.ok && validateAVL(deleteRL, why, true), "delete RL fixture loads balanced");
    int deleteRLChild = findKey(deleteRL, 70)->id;
    int deleteRLPivot = findKey(deleteRL, 50)->id;
    AVLTree::OpResult deleteRLResult = deleteRL.deleteKey(20);
    check(deleteRLResult.ok && deleteRL.lastRotations() == 2,
          "delete RL performs two primitive rotations");
    int deleteRLFirst = firstEvent(deleteRLResult.events, EventType::ROTATE);
    int deleteRLSecond = -1;
    for (int i = deleteRLFirst + 1; i < deleteRLResult.events.size(); i++) {
        if (deleteRLResult.events.at(i).type == EventType::ROTATE) {
            deleteRLSecond = i;
            break;
        }
    }
    check(deleteRLFirst >= 0 && deleteRLResult.events.at(deleteRLFirst).idA == deleteRLChild &&
          deleteRLResult.events.at(deleteRLFirst).extra == RotationType::RL &&
          deleteRLSecond >= 0 && deleteRLResult.events.at(deleteRLSecond).idA == deleteRLPivot &&
          deleteRLResult.events.at(deleteRLSecond).extra == RotationType::RL,
          "delete RL labels both primitives RL in order");
    check(deleteRL.root() != nullptr && static_cast<AVLNode*>(deleteRL.root())->key == 60 &&
          validateAVL(deleteRL, why, true),
          "delete RL leaves a valid tree rooted at 60");

    AVLTree zeroLeft;
    BinaryTree::BuildResult loadZeroLeft =
        zeroLeft.deserialize("50 30 20 10 # # 25 # # 40 # 45 # # 70 # 80 # #");
    check(loadZeroLeft.ok && validateAVL(zeroLeft, why, true), "zero-balance LL fixture loads balanced");
    int zeroLeftPivot = findKey(zeroLeft, 50)->id;
    AVLTree::OpResult zeroLeftResult = zeroLeft.deleteKey(80);
    check(zeroLeftResult.ok && zeroLeft.lastRotations() == 1 &&
          eventCount(zeroLeftResult.events, EventType::ROTATE) == 1 &&
          zeroLeftResult.events.at(firstEvent(zeroLeftResult.events, EventType::ROTATE)).idA == zeroLeftPivot &&
          zeroLeftResult.events.at(firstEvent(zeroLeftResult.events, EventType::ROTATE)).extra == RotationType::LL,
          "left-heavy z with zero-balance left child takes one LL rotation");
    check(validateAVL(zeroLeft, why, true), "zero-balance LL result is valid");

    AVLTree zeroRight;
    BinaryTree::BuildResult loadZeroRight =
        zeroRight.deserialize("50 30 20 # # # 70 60 55 # # 65 # # 80 # 90 # #");
    check(loadZeroRight.ok && validateAVL(zeroRight, why, true), "zero-balance RR fixture loads balanced");
    int zeroRightPivot = findKey(zeroRight, 50)->id;
    AVLTree::OpResult zeroRightResult = zeroRight.deleteKey(20);
    check(zeroRightResult.ok && zeroRight.lastRotations() == 1 &&
          eventCount(zeroRightResult.events, EventType::ROTATE) == 1 &&
          zeroRightResult.events.at(firstEvent(zeroRightResult.events, EventType::ROTATE)).idA == zeroRightPivot &&
          zeroRightResult.events.at(firstEvent(zeroRightResult.events, EventType::ROTATE)).extra == RotationType::RR,
          "right-heavy z with zero-balance right child takes one RR rotation");
    check(validateAVL(zeroRight, why, true), "zero-balance RR result is valid");

    AVLTree cascade;
    BinaryTree::BuildResult loadCascade =
        cascade.deserialize("8 5 3 2 1 # # # 4 # # 7 6 # # # 11 10 9 # # # 12 # #");
    check(loadCascade.ok && validateAVL(cascade, why, true), "cascade fixture loads balanced");
    AVLTree::OpResult cascadeResult = cascade.deleteKey(12);
    check(cascadeResult.ok && cascade.lastRotations() >= 2,
          "cascade delete performs at least two rebalances");
    check(eventCount(cascadeResult.events, EventType::BALANCE_ALERT) >= 2,
          "cascade delete alerts at every unbalanced ancestor");
    check(validateAVL(cascade, why, true), "cascade delete leaves a valid tree");

    AVLTree differential;
    DynArray<int> reference;
    Rng differentialRng(90210u);
    bool differentialOk = true;
    for (int step = 0; step < 500; step++) {
        int operation = differentialRng.range(0, 1);
        int key = differentialRng.range(1, 120);
        if (operation == 0) {
            bool expectedOk = !referenceContains(reference, key);
            AVLTree::OpResult result = differential.insert(key);
            if (result.ok != expectedOk) {
                differentialOk = false;
                break;
            }
            if (result.ok) {
                referenceInsert(reference, key);
            }
        } else {
            bool expectedOk = referenceContains(reference, key);
            AVLTree::OpResult result = differential.deleteKey(key);
            if (result.ok != expectedOk) {
                differentialOk = false;
                break;
            }
            if (result.ok) {
                referenceRemove(reference, key);
            }
        }
        if (!validateAVL(differential, why, true)) {
            differentialOk = false;
            break;
        }
        DynArray<int> keys;
        collectKeys(differential, keys);
        if (!sameArray(keys, reference) || differential.size() != reference.size()) {
            differentialOk = false;
            break;
        }
        for (int k = 1; k <= reference.size(); k++) {
            BSTNode* node = differential.kth(k);
            if (node == nullptr || node->key != reference[k - 1]) {
                differentialOk = false;
                break;
            }
        }
        if (!differentialOk || differential.kth(0) != nullptr ||
            differential.kth(reference.size() + 1) != nullptr) {
            differentialOk = false;
            break;
        }
        for (int i = 0; i < reference.size(); i++) {
            if (differential.rank(reference[i]) != referenceRank(reference, reference[i])) {
                differentialOk = false;
                break;
            }
        }
        int below = reference.size() == 0 ? 0 : reference[0] - 1;
        int above = reference.size() == 0 ? 0 : reference[reference.size() - 1] + 1;
        if (!differentialOk || differential.rank(below) != referenceRank(reference, below) ||
            differential.rank(above) != referenceRank(reference, above)) {
            differentialOk = false;
            break;
        }
        for (int probe = 1; probe <= 120; probe++) {
            if (differential.rank(probe) != referenceRank(reference, probe)) {
                differentialOk = false;
                break;
            }
        }
    }
    check(differentialOk, "500 mixed operations keep kth and rank equal to the reference");
    if (!differentialOk) {
        cout << "  differential checker: " << why << endl;
    }

    AVLTree rangeTree;
    DynArray<int> rangeInput;
    for (int i = 1; i <= 100; i++) {
        rangeInput.push(i);
    }
    Rng rangeRng(5150u);
    rangeRng.shuffle(rangeInput);
    for (int i = 0; i < rangeInput.size(); i++) {
        rangeTree.insert(rangeInput[i]);
    }
    DynArray<int> rangeKeys;
    collectKeys(rangeTree, rangeKeys);
    int bounds[] = {INT_MIN, -1000, 0, 1, 25, 50, 75, 99, 100, 101, 1000, INT_MAX};
    bool rangeOk = true;
    for (int i = 0; i < 12 && rangeOk; i++) {
        for (int j = 0; j < 12; j++) {
            int lo = bounds[i];
            int hi = bounds[j];
            int expected = 0;
            for (int k = 0; k < rangeKeys.size(); k++) {
                if (rangeKeys[k] >= lo && rangeKeys[k] <= hi) {
                    expected++;
                }
            }
            if (rangeTree.countInRange(lo, hi) != expected) {
                rangeOk = false;
                break;
            }
        }
    }
    check(rangeOk, "countInRange matches the reference for all bound pairs");
    check(rangeTree.countInRange(1, 100) == 100 && rangeTree.countInRange(INT_MIN, INT_MAX) == 100,
          "countInRange covers the full range without overflow");
    check(rangeTree.countInRange(60, 40) == 0 && rangeTree.countInRange(1000, 2000) == 0,
          "countInRange returns zero for inverted and empty ranges");
    AVLTree emptyRange;
    check(emptyRange.countInRange(INT_MIN, INT_MAX) == 0,
          "countInRange on an empty tree is zero");

    AVLTree manual;
    manual.insert(30);
    manual.insert(20);
    manual.insert(40);
    int manualRootId = manual.root()->id;
    int manualLeftId = findKey(manual, 20)->id;
    int manualRightId = findKey(manual, 40)->id;
    DynArray<int> manualIdsBefore;
    collectIds(manual, manualIdsBefore);
    mergeSort(manualIdsBefore);
    AVLTree::OpResult manualLeft = manual.rotateLeft(manualRootId);
    check(manualLeft.ok && manual.root() != nullptr && manual.root()->id == manualRightId &&
          manual.root()->parent == nullptr,
          "manual rotateLeft moves the right child up and fixes the root");
    check(findKey(manual, 30) != nullptr && findKey(manual, 30)->parent == findKey(manual, 40) &&
          findKey(manual, 40)->left == findKey(manual, 30) &&
          findKey(manual, 20)->parent == findKey(manual, 30),
          "manual rotateLeft keeps every parent link");
    check(eventCount(manualLeft.events, EventType::ROTATE) == 1 &&
          manualLeft.events.at(firstEvent(manualLeft.events, EventType::ROTATE)).idA == manualRootId &&
          manualLeft.events.at(firstEvent(manualLeft.events, EventType::ROTATE)).idB == manualRightId &&
          manualLeft.events.at(firstEvent(manualLeft.events, EventType::ROTATE)).key == 0 &&
          manualLeft.events.at(firstEvent(manualLeft.events, EventType::ROTATE)).extra == RotationType::RR,
          "manual rotateLeft emits a standalone RR rotation");
    check(firstEvent(manualLeft.events, EventType::ROTATE) <
          firstEvent(manualLeft.events, EventType::UPDATE_HEIGHT) &&
          firstEvent(manualLeft.events, EventType::UPDATE_HEIGHT) >= 0 &&
          eventCount(manualLeft.events, EventType::UPDATE_SIZE) > 0 &&
          manualLeft.events.at(manualLeft.events.size() - 1).type == EventType::DONE,
          "manual rotateLeft emits ROTATE before UPDATE events and ends with DONE");
    check(!manual.isBalanced(), "a manual rotation may leave the tree unbalanced");
    check(validateAVL(manual, why, false), "manual rotateLeft keeps stored stats correct");
    DynArray<int> manualIdsAfter;
    collectIds(manual, manualIdsAfter);
    mergeSort(manualIdsAfter);
    check(sameArray(manualIdsBefore, manualIdsAfter), "manual rotateLeft creates and destroys no nodes");
    AVLTree::OpResult manualRight = manual.rotateRight(findKey(manual, 40)->id);
    check(manualRight.ok && manual.root() != nullptr && static_cast<AVLNode*>(manual.root())->key == 30,
          "manual rotateRight restores the earlier shape");
    check(validateAVL(manual, why, false), "manual rotateRight keeps stored stats correct");

    AVLTree::OpResult missingRotate = manual.rotateLeft(999999);
    check(!missingRotate.ok && missingRotate.message == "node 999999 not found" &&
          missingRotate.events.at(missingRotate.events.size() - 1).type == EventType::DONE,
          "manual rotateLeft rejects a missing id");
    AVLTree::OpResult noRight = manual.rotateLeft(manualLeftId);
    check(!noRight.ok && noRight.message == "node " + to_string(manualLeftId) + " has no right child",
          "manual rotateLeft rejects a node without a right child");
    AVLTree::OpResult noLeft = manual.rotateRight(manualLeftId);
    check(!noLeft.ok && noLeft.message == "node " + to_string(manualLeftId) + " has no left child",
          "manual rotateRight rejects a node without a left child");

    AVLTree loaded;
    BinaryTree::BuildResult loadResult = loaded.deserialize("2 1 # # 3 # #");
    check(loadResult.ok && loaded.height() == 2 && loaded.size() == 3 &&
          validateAVL(loaded, why, true),
          "deserialize recomputes height and size on load");
    DynArray<int> preorderInput;
    int preorderData[] = {4, 2, 1, 3, 6, 5, 7};
    for (int i = 0; i < 7; i++) {
        preorderInput.push(preorderData[i]);
    }
    DynArray<int> inorderInput;
    for (int i = 1; i <= 7; i++) {
        inorderInput.push(i);
    }
    AVLTree fromPreIn;
    BinaryTree::BuildResult preInResult = fromPreIn.buildFromPreorderInorder(preorderInput, inorderInput);
    check(preInResult.ok && fromPreIn.height() == 3 && fromPreIn.size() == 7 &&
          validateAVL(fromPreIn, why, true),
          "buildFromPreorderInorder recomputes stats");
    DynArray<int> postorderInput;
    int postorderData[] = {1, 3, 2, 5, 7, 6, 4};
    for (int i = 0; i < 7; i++) {
        postorderInput.push(postorderData[i]);
    }
    AVLTree fromPostIn;
    BinaryTree::BuildResult postInResult = fromPostIn.buildFromPostorderInorder(postorderInput, inorderInput);
    check(postInResult.ok && fromPostIn.height() == 3 && fromPostIn.size() == 7 &&
          validateAVL(fromPostIn, why, true),
          "buildFromPostorderInorder recomputes stats");

    AVLNode* corruptedRoot = static_cast<AVLNode*>(fromPreIn.root());
    corruptedRoot->height = 99;
    corruptedRoot->size = 99;
    findKey(fromPreIn, 1)->height = 0;
    check(!validateAVL(fromPreIn, why, true), "checker rejects corrupted stored stats");
    fromPreIn.recomputeStats();
    check(validateAVL(fromPreIn, why, true) && corruptedRoot->height == 3 && corruptedRoot->size == 7,
          "recomputeStats repairs corrupted height and size");

    AVLTree balancedTree;
    int balancedInput[] = {4, 2, 6, 1, 3, 5, 7};
    insertAll(balancedTree, balancedInput, 7);
    check(balancedTree.isBalanced(), "isBalanced accepts a balanced tree");
    AVLTree chainTree;
    BinaryTree::BuildResult chainLoad = chainTree.deserialize("1 # 2 # 3 # #");
    check(chainLoad.ok && !chainTree.isBalanced() &&
          validateAVL(chainTree, why, false),
          "isBalanced rejects a right chain");
    static_cast<AVLNode*>(balancedTree.root())->height = 99;
    check(balancedTree.isBalanced(), "isBalanced ignores a corrupted stored height");
    check(!validateAVL(balancedTree, why, true), "checker still sees the corrupted height");
    balancedTree.recomputeStats();
    static_cast<AVLNode*>(balancedTree.root())->size = 99;
    check(balancedTree.isBalanced(), "isBalanced ignores a corrupted stored size");
    balancedTree.recomputeStats();
    check(validateAVL(balancedTree, why, true), "recomputeStats fixes the size again");

    AVLTree idTree;
    int idInput[] = {50, 30, 70, 20, 40, 60, 80};
    insertAll(idTree, idInput, 7);
    int idMaximum = 0;
    for (int i = 0; i < 7; i++) {
        int id = findKey(idTree, idInput[i])->id;
        idMaximum = id > idMaximum ? id : idMaximum;
    }
    idTree.deleteKey(30);
    idTree.insert(35);
    check(findKey(idTree, 35) != nullptr && findKey(idTree, 35)->id > idMaximum,
          "ids after delete and re-insert are never reused");
    int idAfterReinsert = findKey(idTree, 35)->id;
    idTree.clear();
    idTree.insert(5);
    check(idTree.root() != nullptr && idTree.root()->id > idAfterReinsert,
          "ids continue after clear");
    AVLTree rotateIds;
    rotateIds.insert(30);
    rotateIds.insert(20);
    rotateIds.insert(40);
    DynArray<int> rotateIdsBefore;
    collectIds(rotateIds, rotateIdsBefore);
    mergeSort(rotateIdsBefore);
    rotateIds.rotateLeft(rotateIds.root()->id);
    DynArray<int> rotateIdsAfter;
    collectIds(rotateIds, rotateIdsAfter);
    mergeSort(rotateIdsAfter);
    check(sameArray(rotateIdsBefore, rotateIdsAfter), "rotations preserve the full id set");

    AVLTree recordingOn(true);
    AVLTree recordingOff(false);
    bool parityOk = true;
    for (int step = 0; step < 40; step++) {
        int key = (step * 37) % 200;
        if (step % 3 == 0) {
            AVLTree::OpResult onDelete = recordingOn.deleteKey(key);
            AVLTree::OpResult offDelete = recordingOff.deleteKey(key);
            parityOk = parityOk && onDelete.ok == offDelete.ok &&
                       onDelete.message == offDelete.message && !onDelete.events.empty() &&
                       offDelete.events.empty();
        } else {
            AVLTree::OpResult onInsert = recordingOn.insert(key);
            AVLTree::OpResult offInsert = recordingOff.insert(key);
            parityOk = parityOk && onInsert.ok == offInsert.ok &&
                       onInsert.message == offInsert.message && !onInsert.events.empty() &&
                       offInsert.events.empty();
        }
        parityOk = parityOk && recordingOn.serialize() == recordingOff.serialize() &&
                   recordingOn.rotationCount() == recordingOff.rotationCount() &&
                   recordingOn.lastRotations() == recordingOff.lastRotations() &&
                   recordingOn.height() == recordingOff.height() &&
                   recordingOn.size() == recordingOff.size() &&
                   validateAVL(recordingOn, why, true) && validateAVL(recordingOff, why, true);
    }
    DynArray<int> onKeys;
    DynArray<int> offKeys;
    collectKeys(recordingOn, onKeys);
    collectKeys(recordingOff, offKeys);
    BSTNode* onKth = recordingOn.kth(1);
    BSTNode* offKth = recordingOff.kth(1);
    parityOk = parityOk && sameArray(onKeys, offKeys) &&
               (onKth == nullptr) == (offKth == nullptr) &&
               (onKth == nullptr || onKth->key == offKth->key) &&
               recordingOn.rank(75) == recordingOff.rank(75) &&
               recordingOn.countInRange(20, 80) == recordingOff.countInRange(20, 80);
    check(parityOk, "recording off matches recording on with empty logs");

    AVLTree counters;
    counters.insert(30);
    counters.insert(20);
    counters.insert(10);
    check(counters.rotationCount() == 1 && counters.lastRotations() == 1,
          "counters start at one rotation after LL");
    counters.insert(25);
    check(counters.rotationCount() == 1 && counters.lastRotations() == 0,
          "lastRotations resets when an insert needs no rotation");
    counters.insert(15);
    check(counters.rotationCount() >= 1, "rotationCount is cumulative");
    int cumulative = counters.rotationCount();
    AVLTree doubleCounters;
    doubleCounters.insert(30);
    doubleCounters.insert(10);
    doubleCounters.insert(20);
    check(doubleCounters.rotationCount() == 2 && doubleCounters.lastRotations() == 2,
          "a double rotation adds two to both counters");
    doubleCounters.insert(40);
    check(doubleCounters.rotationCount() == 2 && doubleCounters.lastRotations() == 0,
          "an insert without rotation leaves the cumulative count alone");
    doubleCounters.rotateRight(doubleCounters.root()->id);
    check(doubleCounters.rotationCount() == 3 && doubleCounters.lastRotations() == 1,
          "manual rotations count as one primitive rotation");
    check(cumulative >= 1, "cumulative count survives later operations");

    AVLTree big(false);
    bool bigOk = true;
    clock_t insertStart = clock();
    for (int i = 1; i <= 10000; i++) {
        AVLTree::OpResult result = big.insert(i);
        bigOk = bigOk && result.ok && result.events.empty();
        if (i % 1000 == 0) {
            bigOk = bigOk && validateAVL(big, why, true);
        }
    }
    clock_t insertFinish = clock();
    double insertElapsed = static_cast<double>(insertFinish - insertStart) /
                           static_cast<double>(CLOCKS_PER_SEC);
    cout << "10,000 sorted AVL inserts (recording off): " << insertElapsed << " s" << endl;
    check(bigOk && big.height() == 14 && big.size() == 10000 && big.rotationCount() > 0,
          "10,000 sorted AVL inserts give height 14");
    check(validateAVL(big, why, true), "10,000-node AVL tree passes the checker");
    bool drainOk = true;
    clock_t drainStart = clock();
    for (int i = 1; i <= 10000; i++) {
        AVLTree::OpResult result = big.deleteKey(i);
        drainOk = drainOk && result.ok;
        if (i % 1000 == 0) {
            drainOk = drainOk && validateAVL(big, why, true);
        }
    }
    clock_t drainFinish = clock();
    double drainElapsed = static_cast<double>(drainFinish - drainStart) /
                          static_cast<double>(CLOCKS_PER_SEC);
    cout << "10,000 sorted AVL deletes (recording off): " << drainElapsed << " s" << endl;
    check(drainOk && big.empty() && big.size() == 0,
          "10,000 sorted deletes drain the AVL tree");

    AVLTree apiTree;
    BinaryTree::BuildResult apiLoad = apiTree.deserialize("50 30 20 # # 40 # # 70 60 # # 80 # #");
    check(apiLoad.ok, "BST API fixture loads");
    BST& base = apiTree;
    BST::SearchResult apiSearch = base.search(60);
    check(apiSearch.found != nullptr && apiSearch.found->key == 60,
          "search works on a rotated AVL tree through the BST API");
    BST::RangeResult apiRange = base.rangeQuery(30, 60);
    int expectedRangeData[] = {30, 40, 50, 60};
    check(sameArray(apiRange.keys, makeArray(expectedRangeData, 4)),
          "rangeQuery works on a rotated AVL tree through the BST API");
    check(base.successor(50) != nullptr && base.successor(50)->key == 60 &&
          base.predecessor(50) != nullptr && base.predecessor(50)->key == 40,
          "successor and predecessor work through the BST API");
    check(base.kthSmallest(3) != nullptr && base.kthSmallest(3)->key == 40 &&
          base.kthSmallest(8) == nullptr,
          "kthSmallest works through the BST API");
    check(base.lcaBST(20, 40) != nullptr && base.lcaBST(20, 40)->key == 30 &&
          base.lcaBST(20, 80) != nullptr && base.lcaBST(20, 80)->key == 50,
          "lcaBST works on a rotated AVL tree");

    return failed;
}
