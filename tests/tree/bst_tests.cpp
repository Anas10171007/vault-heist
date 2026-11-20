#include <ctime>
#include <iostream>
#include <string>

#include "model/tree/BST.h"
#include "util/DynArray.h"
#include "util/Rng.h"
#include "util/Stack.h"

using namespace std;
using namespace vh;

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

static bool referenceContains(const DynArray<int>& values, int key) {
    int lo = 0;
    int hi = values.size() - 1;
    while (lo <= hi) {
        int mid = (lo + hi) / 2;
        if (values[mid] == key) {
            return true;
        }
        if (values[mid] < key) {
            lo = mid + 1;
        } else {
            hi = mid - 1;
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

static BSTNode* findKey(const BST& tree, int key) {
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

static int expectedPathLength(const BST& tree, int key) {
    int count = 0;
    TreeNode* current = tree.root();
    while (current != nullptr) {
        count++;
        BSTNode* node = static_cast<BSTNode*>(current);
        if (key == node->key) {
            break;
        }
        current = key < node->key ? node->left : node->right;
    }
    return count;
}

static void collectKeys(const BST& tree, DynArray<int>& keys) {
    Stack<TreeNode*> pending;
    TreeNode* current = tree.root();
    while (current != nullptr || !pending.empty()) {
        while (current != nullptr) {
            pending.push(current);
            current = current->left;
        }
        current = pending.pop();
        keys.push(static_cast<BSTNode*>(current)->key);
        current = current->right;
    }
}

// Independent checker: iterative inorder, ordering, key/value sync, parent links,
// and node uniqueness (shared nodes would also break a parent link).
static bool validateBST(const BST& tree, string& why) {
    DynArray<TreeNode*> nodes;
    Stack<TreeNode*> pending;
    TreeNode* current = tree.root();
    int steps = 0;
    const int limit = 200000;
    while (current != nullptr || !pending.empty()) {
        while (current != nullptr) {
            pending.push(current);
            current = current->left;
            steps++;
            if (steps > limit) {
                why = "traversal did not terminate";
                return false;
            }
        }
        current = pending.pop();
        nodes.push(current);
        current = current->right;
    }
    for (int i = 0; i < nodes.size(); i++) {
        BSTNode* node = static_cast<BSTNode*>(nodes[i]);
        if (node->key != node->value) {
            why = "key and value differ";
            return false;
        }
        if (node->left != nullptr) {
            if (node->left->parent != node) {
                why = "left child parent link is wrong";
                return false;
            }
            if (static_cast<BSTNode*>(node->left)->key >= node->key) {
                why = "left child is not smaller";
                return false;
            }
        }
        if (node->right != nullptr) {
            if (node->right->parent != node) {
                why = "right child parent link is wrong";
                return false;
            }
            if (static_cast<BSTNode*>(node->right)->key <= node->key) {
                why = "right child is not larger";
                return false;
            }
        }
        if (i > 0 && static_cast<BSTNode*>(nodes[i - 1])->key >= node->key) {
            why = "inorder is not strictly ascending";
            return false;
        }
    }
    if (tree.root() != nullptr && tree.root()->parent != nullptr) {
        why = "root has a parent";
        return false;
    }
    for (int i = 0; i < nodes.size(); i++) {
        for (int j = i + 1; j < nodes.size(); j++) {
            if (nodes[i] == nodes[j]) {
                why = "a node is shared";
                return false;
            }
            if (nodes[i]->id == nodes[j]->id) {
                why = "two nodes share an id";
                return false;
            }
        }
    }
    return true;
}

static int countEvents(const EventLog& log, int type) {
    int count = 0;
    for (int i = 0; i < log.size(); i++) {
        if (log.at(i).type == type) {
            count++;
        }
    }
    return count;
}

int runBSTTests() {
    int failed = 0;
    auto check = [&](bool ok, const char* name) {
        cout << (ok ? "PASS " : "FAIL ") << name << endl;
        if (!ok) {
            failed++;
        }
    };
    string why;

    BST empty;
    check(empty.findMin() == nullptr && empty.findMax() == nullptr, "empty BST has no min or max");
    check(empty.size() == 0 && empty.empty() && empty.height() == 0, "empty BST measures zero");

    BST corruptOrder;
    corruptOrder.insert(50);
    corruptOrder.insert(30);
    corruptOrder.insert(70);
    static_cast<BSTNode*>(corruptOrder.root())->setKey(10);
    check(!validateBST(corruptOrder, why), "checker rejects a tree with broken ordering");

    BST corruptParent;
    corruptParent.insert(50);
    corruptParent.insert(30);
    corruptParent.root()->left->parent = nullptr;
    check(!validateBST(corruptParent, why), "checker rejects a broken parent link");

    BST corruptSync;
    corruptSync.insert(50);
    static_cast<BSTNode*>(corruptSync.root())->value = 99;
    check(!validateBST(corruptSync, why), "checker rejects a key/value mismatch");

    BST first;
    BST::OpResult firstInsert = first.insert(50);
    check(firstInsert.ok && firstInsert.message.empty(), "insert into an empty tree succeeds");
    check(first.root() != nullptr && static_cast<BSTNode*>(first.root())->key == 50, "first insert becomes the root");
    check(countEvents(firstInsert.events, EventType::INSERT) == 1, "empty insert emits one INSERT");
    check(firstInsert.events.at(0).idB == 0 && firstInsert.events.at(0).side == Side::NONE &&
          firstInsert.events.at(0).key == 50,
          "root INSERT has parent 0 and side NONE");
    check(firstInsert.events.at(firstInsert.events.size() - 1).type == EventType::DONE,
          "insert ends with DONE");
    check(validateBST(first, why), "checker accepts the root");

    BST asc;
    bool ascOk = true;
    for (int i = 10; i <= 40; i += 10) {
        BST::OpResult result = asc.insert(i);
        ascOk = ascOk && result.ok && validateBST(asc, why);
    }
    check(ascOk, "ascending inserts keep the BST valid after every insert");
    DynArray<int> ascKeys;
    collectKeys(asc, ascKeys);
    int ascExpectedData[] = {10, 20, 30, 40};
    check(sameArray(ascKeys, makeArray(ascExpectedData, 4)), "ascending inserts produce sorted inorder");
    check(asc.height() == 4, "ascending inserts build a right chain");

    BST desc;
    bool descOk = true;
    for (int i = 40; i >= 10; i -= 10) {
        BST::OpResult result = desc.insert(i);
        descOk = descOk && result.ok && validateBST(desc, why);
    }
    check(descOk, "descending inserts keep the BST valid after every insert");
    DynArray<int> descKeys;
    collectKeys(desc, descKeys);
    int descExpectedData[] = {10, 20, 30, 40};
    check(sameArray(descKeys, makeArray(descExpectedData, 4)), "descending inserts produce sorted inorder");
    check(desc.height() == 4, "descending inserts build a left chain");

    BST mixed;
    int mixedInput[] = {50, 30, 70, 20, 40, 60, 80};
    bool mixedOk = true;
    for (int i = 0; i < 7; i++) {
        BST::OpResult result = mixed.insert(mixedInput[i]);
        mixedOk = mixedOk && result.ok && validateBST(mixed, why);
    }
    check(mixedOk, "mixed inserts keep the BST valid after every insert");
    DynArray<int> mixedKeys;
    collectKeys(mixed, mixedKeys);
    int mixedExpectedData[] = {20, 30, 40, 50, 60, 70, 80};
    check(sameArray(mixedKeys, makeArray(mixedExpectedData, 7)), "mixed inserts produce sorted inorder");
    check(mixed.height() == 3 && mixed.size() == 7, "mixed inserts measure height and size");

    DynArray<int> shuffled;
    for (int i = 1; i <= 200; i++) {
        shuffled.push(i);
    }
    Rng rng(20261120u);
    rng.shuffle(shuffled);
    BST randomTree;
    bool randomOk = true;
    for (int i = 0; i < shuffled.size(); i++) {
        BST::OpResult result = randomTree.insert(shuffled[i]);
        randomOk = randomOk && result.ok && validateBST(randomTree, why);
    }
    check(randomOk, "200 seeded random inserts keep the BST valid after every insert");
    check(randomTree.size() == 200, "200 random inserts produce 200 nodes");
    DynArray<int> randomKeys;
    collectKeys(randomTree, randomKeys);
    DynArray<int> expectedOneToTwoHundred;
    for (int i = 1; i <= 200; i++) {
        expectedOneToTwoHundred.push(i);
    }
    check(sameArray(randomKeys, expectedOneToTwoHundred), "200 random inserts produce the sorted inorder");

    BST eventTree;
    eventTree.insert(50);
    eventTree.insert(30);
    BST::OpResult eventInsert = eventTree.insert(20);
    check(countEvents(eventInsert.events, EventType::COMPARE) == 2, "insert compares along the path");
    check(eventInsert.events.at(0).extra == -1 && eventInsert.events.at(1).extra == -1,
          "COMPARE extra reports the searched key as smaller");
    check(eventInsert.events.at(eventInsert.events.size() - 2).type == EventType::INSERT &&
          eventInsert.events.at(eventInsert.events.size() - 1).type == EventType::DONE,
          "insert emits INSERT immediately before DONE");

    BST dup;
    dup.insert(50);
    dup.insert(30);
    dup.insert(70);
    string dupBefore = dup.serialize();
    int dupSize = dup.size();
    BST::OpResult duplicate = dup.insert(30);
    check(!duplicate.ok && duplicate.message == "duplicate key 30 rejected",
          "duplicate insert is rejected with the exact message");
    check(dup.size() == dupSize && dup.serialize() == dupBefore, "duplicate insert leaves the tree unchanged");
    check(countEvents(duplicate.events, EventType::INSERT) == 0, "duplicate insert emits no INSERT");
    check(duplicate.events.at(duplicate.events.size() - 1).type == EventType::DONE,
          "duplicate insert ends with DONE");
    const Event& duplicateCompare = duplicate.events.at(duplicate.events.size() - 2);
    check(duplicateCompare.type == EventType::COMPARE && duplicateCompare.extra == 0,
          "duplicate search ends on an equal comparison");

    BST searchTree;
    searchTree.insert(50);
    searchTree.insert(30);
    searchTree.insert(70);
    searchTree.insert(20);
    searchTree.insert(40);
    BST::SearchResult found = searchTree.search(40);
    check(found.found != nullptr && found.found->key == 40, "search finds an existing key");
    check(countEvents(found.events, EventType::COMPARE) == expectedPathLength(searchTree, 40),
          "search emits one COMPARE per path node");
    check(found.events.at(found.events.size() - 1).type == EventType::DONE, "search ends with DONE");
    BST::SearchResult missing = searchTree.search(35);
    check(missing.found == nullptr, "search reports a missing key");
    check(countEvents(missing.events, EventType::COMPARE) == expectedPathLength(searchTree, 35),
          "missing search compares every node on the path");
    BST emptySearch;
    BST::SearchResult emptySearchResult = emptySearch.search(1);
    check(emptySearchResult.found == nullptr && countEvents(emptySearchResult.events, EventType::COMPARE) == 0,
          "search on an empty tree compares nothing");

    BST leafTree;
    leafTree.insert(50);
    leafTree.insert(30);
    leafTree.insert(70);
    int leafId = findKey(leafTree, 30)->id;
    int leafRootId = leafTree.root()->id;
    BST::OpResult leafDelete = leafTree.deleteKey(30);
    check(leafDelete.ok && findKey(leafTree, 30) == nullptr, "leaf delete removes the key");
    const Event& leafUnlink = leafDelete.events.at(leafDelete.events.size() - 2);
    check(leafUnlink.type == EventType::UNLINK && leafUnlink.idA == leafId &&
          leafUnlink.idB == leafRootId && leafUnlink.side == Side::LEFT &&
          leafUnlink.key == 30 && leafUnlink.extra == 0,
          "leaf UNLINK carries node, parent, side, key and no child");
    check(leafDelete.events.at(leafDelete.events.size() - 1).type == EventType::DONE, "delete ends with DONE");
    check(validateBST(leafTree, why), "checker accepts the tree after a leaf delete");

    BST leftChildTree;
    leftChildTree.insert(50);
    leftChildTree.insert(30);
    leftChildTree.insert(20);
    int leftRemovedId = findKey(leftChildTree, 30)->id;
    int leftChildId = findKey(leftChildTree, 20)->id;
    BST::OpResult leftDelete = leftChildTree.deleteKey(30);
    const Event& leftUnlink = leftDelete.events.at(leftDelete.events.size() - 2);
    check(leftDelete.ok && leftUnlink.type == EventType::UNLINK && leftUnlink.idA == leftRemovedId &&
          leftUnlink.extra == leftChildId && leftUnlink.side == Side::LEFT,
          "one-left-child UNLINK carries the bypassed child id");
    check(findKey(leftChildTree, 20) != nullptr && findKey(leftChildTree, 20)->parent == leftChildTree.root(),
          "bypassed left child is relinked to the parent");
    check(validateBST(leftChildTree, why), "checker accepts the tree after a one-left-child delete");

    BST rightChildTree;
    rightChildTree.insert(50);
    rightChildTree.insert(30);
    rightChildTree.insert(40);
    int rightRemovedId = findKey(rightChildTree, 30)->id;
    int rightChildId = findKey(rightChildTree, 40)->id;
    BST::OpResult rightDelete = rightChildTree.deleteKey(30);
    const Event& rightUnlink = rightDelete.events.at(rightDelete.events.size() - 2);
    check(rightDelete.ok && rightUnlink.type == EventType::UNLINK && rightUnlink.idA == rightRemovedId &&
          rightUnlink.extra == rightChildId && rightUnlink.side == Side::LEFT,
          "one-right-child UNLINK carries the bypassed child id");
    check(findKey(rightChildTree, 40) != nullptr && findKey(rightChildTree, 40)->parent == rightChildTree.root(),
          "bypassed right child is relinked to the parent");
    check(validateBST(rightChildTree, why), "checker accepts the tree after a one-right-child delete");

    BST immediate;
    int immediateInput[] = {50, 30, 70, 20, 40, 60, 80};
    for (int i = 0; i < 7; i++) {
        immediate.insert(immediateInput[i]);
    }
    int immediateTargetId = findKey(immediate, 70)->id;
    int immediateSuccId = findKey(immediate, 80)->id;
    BST::OpResult immediateDelete = immediate.deleteKey(70);
    int immediateCount = immediateDelete.events.size();
    check(immediateDelete.ok && immediateCount >= 4, "two-child delete succeeds");
    check(immediateDelete.events.at(immediateCount - 4).type == EventType::HIGHLIGHT_SUBTREE &&
          immediateDelete.events.at(immediateCount - 4).idA == immediateSuccId &&
          immediateDelete.events.at(immediateCount - 4).extra == HighlightMode::RESULT,
          "two-child delete highlights the immediate successor");
    check(immediateDelete.events.at(immediateCount - 3).type == EventType::SWAP_VALUE &&
          immediateDelete.events.at(immediateCount - 3).idA == immediateTargetId &&
          immediateDelete.events.at(immediateCount - 3).idB == immediateSuccId &&
          immediateDelete.events.at(immediateCount - 3).oldValue == 70 &&
          immediateDelete.events.at(immediateCount - 3).newValue == 80,
          "SWAP_VALUE carries both ids and both old values");
    check(immediateDelete.events.at(immediateCount - 2).type == EventType::UNLINK &&
          immediateDelete.events.at(immediateCount - 2).idA == immediateSuccId,
          "successor is unlinked after the swap");
    check(immediateDelete.events.at(immediateCount - 1).type == EventType::DONE,
          "two-child delete ends with DONE");
    check(immediate.findById(immediateTargetId) != nullptr &&
          static_cast<BSTNode*>(immediate.findById(immediateTargetId))->key == 80 &&
          immediate.findById(immediateSuccId) == nullptr,
          "the target id survives and the successor id is gone");
    check(findKey(immediate, 70) == nullptr, "the old target key is gone");
    check(validateBST(immediate, why), "checker accepts the tree after the immediate-successor delete");

    BST deeper;
    int deeperInput[] = {50, 30, 70, 20, 40, 60, 80, 65};
    for (int i = 0; i < 8; i++) {
        deeper.insert(deeperInput[i]);
    }
    int deeperTargetId = findKey(deeper, 50)->id;
    int deeperSuccId = findKey(deeper, 60)->id;
    int deeperChildId = findKey(deeper, 65)->id;
    int deeperParentId = findKey(deeper, 70)->id;
    BST::OpResult deeperDelete = deeper.deleteKey(50);
    int deeperCount = deeperDelete.events.size();
    check(deeperDelete.ok, "deeper two-child delete succeeds");
    check(deeperDelete.events.at(deeperCount - 4).type == EventType::HIGHLIGHT_SUBTREE &&
          deeperDelete.events.at(deeperCount - 4).idA == deeperSuccId,
          "deeper delete highlights the successor");
    check(deeperDelete.events.at(deeperCount - 3).type == EventType::SWAP_VALUE &&
          deeperDelete.events.at(deeperCount - 3).idA == deeperTargetId &&
          deeperDelete.events.at(deeperCount - 3).idB == deeperSuccId,
          "deeper SWAP_VALUE uses both node ids");
    const Event& deeperUnlink = deeperDelete.events.at(deeperCount - 2);
    check(deeperUnlink.type == EventType::UNLINK && deeperUnlink.idA == deeperSuccId &&
          deeperUnlink.idB == deeperParentId && deeperUnlink.side == Side::LEFT &&
          deeperUnlink.extra == deeperChildId,
          "deeper successor UNLINK carries its right child for undo");
    check(findKey(deeper, 65) != nullptr && findKey(deeper, 65)->parent == findKey(deeper, 70),
          "successor's right child is relinked to its parent");
    check(deeper.findById(deeperTargetId) != nullptr &&
          static_cast<BSTNode*>(deeper.findById(deeperTargetId))->key == 60 &&
          findKey(deeper, 50) == nullptr,
          "deeper target id survives with the successor key");
    check(validateBST(deeper, why), "checker accepts the tree after the deeper delete");

    BST rootLeaf;
    rootLeaf.insert(50);
    int rootLeafId = rootLeaf.root()->id;
    BST::OpResult rootLeafDelete = rootLeaf.deleteKey(50);
    const Event& rootLeafUnlink = rootLeafDelete.events.at(rootLeafDelete.events.size() - 2);
    check(rootLeafDelete.ok && rootLeaf.empty(), "root leaf delete empties the tree");
    check(rootLeafUnlink.type == EventType::UNLINK && rootLeafUnlink.idA == rootLeafId &&
          rootLeafUnlink.idB == 0 && rootLeafUnlink.side == Side::NONE && rootLeafUnlink.extra == 0,
          "root leaf UNLINK has parent 0 and side NONE");

    BST rootOne;
    rootOne.insert(50);
    rootOne.insert(30);
    int rootOneOldId = rootOne.root()->id;
    int rootOneChildId = findKey(rootOne, 30)->id;
    BST::OpResult rootOneDelete = rootOne.deleteKey(50);
    const Event& rootOneUnlink = rootOneDelete.events.at(rootOneDelete.events.size() - 2);
    check(rootOneDelete.ok && rootOne.size() == 1 && rootOne.root()->id == rootOneChildId &&
          rootOne.root()->parent == nullptr,
          "root with one child is replaced by the child");
    check(rootOneUnlink.type == EventType::UNLINK && rootOneUnlink.idA == rootOneOldId &&
          rootOneUnlink.idB == 0 && rootOneUnlink.side == Side::NONE &&
          rootOneUnlink.extra == rootOneChildId,
          "root one-child UNLINK carries the new root id");
    check(validateBST(rootOne, why), "checker accepts the tree after a root one-child delete");

    BST rootTwo;
    for (int i = 0; i < 7; i++) {
        rootTwo.insert(mixedInput[i]);
    }
    int rootTwoOldId = rootTwo.root()->id;
    int rootTwoSuccId = findKey(rootTwo, 60)->id;
    BST::OpResult rootTwoDelete = rootTwo.deleteKey(50);
    check(rootTwoDelete.ok && rootTwo.root()->id == rootTwoOldId &&
          static_cast<BSTNode*>(rootTwo.root())->key == 60,
          "root with two children keeps its id and takes the successor key");
    check(rootTwo.findById(rootTwoSuccId) == nullptr, "root two-child delete removes the successor node");
    check(validateBST(rootTwo, why), "checker accepts the tree after a root two-child delete");

    BST missingDelete;
    missingDelete.insert(50);
    missingDelete.insert(30);
    missingDelete.insert(70);
    string missingBefore = missingDelete.serialize();
    int missingSize = missingDelete.size();
    BST::OpResult missingDeleteResult = missingDelete.deleteKey(999);
    check(!missingDeleteResult.ok && missingDeleteResult.message == "key 999 not found",
          "missing delete reports the exact message");
    check(missingDelete.size() == missingSize && missingDelete.serialize() == missingBefore,
          "missing delete leaves the tree unchanged");
    check(missingDeleteResult.events.at(missingDeleteResult.events.size() - 1).type == EventType::DONE,
          "missing delete ends with DONE");

    BST drain;
    DynArray<int> drainKeys;
    for (int i = 1; i <= 50; i++) {
        drain.insert(i * 2);
        drainKeys.push(i * 2);
    }
    Rng drainRng(777u);
    drainRng.shuffle(drainKeys);
    bool drainOk = true;
    for (int i = 0; i < drainKeys.size(); i++) {
        BST::OpResult result = drain.deleteKey(drainKeys[i]);
        drainOk = drainOk && result.ok && validateBST(drain, why);
    }
    check(drainOk && drain.empty() && drain.size() == 0, "deleting every key drains the tree");

    BST minMax;
    check(minMax.findMin() == nullptr && minMax.findMax() == nullptr, "empty min and max are null");
    for (int i = 0; i < 7; i++) {
        minMax.insert(mixedInput[i]);
    }
    check(minMax.findMin() != nullptr && minMax.findMin()->key == 20 &&
          minMax.findMax() != nullptr && minMax.findMax()->key == 80,
          "findMin and findMax return the extremes");
    minMax.deleteKey(20);
    check(minMax.findMin()->key == 30, "findMin follows deletions");
    minMax.deleteKey(80);
    check(minMax.findMax()->key == 70, "findMax follows deletions");

    BST successorTree;
    int successorInput[] = {10, 20, 30, 40};
    for (int i = 0; i < 4; i++) {
        successorTree.insert(successorInput[i]);
    }
    check(successorTree.successor(20) != nullptr && successorTree.successor(20)->key == 30,
          "successor of a present key");
    check(successorTree.successor(25) != nullptr && successorTree.successor(25)->key == 30,
          "successor works for an absent key");
    check(successorTree.successor(5) != nullptr && successorTree.successor(5)->key == 10,
          "successor below the minimum is the minimum");
    check(successorTree.successor(40) == nullptr && successorTree.successor(45) == nullptr,
          "the maximum has no successor");
    check(successorTree.predecessor(30) != nullptr && successorTree.predecessor(30)->key == 20,
          "predecessor of a present key");
    check(successorTree.predecessor(25) != nullptr && successorTree.predecessor(25)->key == 20,
          "predecessor works for an absent key");
    check(successorTree.predecessor(45) != nullptr && successorTree.predecessor(45)->key == 40,
          "predecessor above the maximum is the maximum");
    check(successorTree.predecessor(10) == nullptr && successorTree.predecessor(5) == nullptr,
          "the minimum has no predecessor");
    BST emptyOrder;
    check(emptyOrder.successor(1) == nullptr && emptyOrder.predecessor(1) == nullptr,
          "empty tree has no successor or predecessor");
    bool neighbourOk = true;
    DynArray<int> neighbourKeys;
    collectKeys(randomTree, neighbourKeys);
    for (int i = 0; i < neighbourKeys.size(); i++) {
        BSTNode* next = randomTree.successor(neighbourKeys[i]);
        BSTNode* previous = randomTree.predecessor(neighbourKeys[i]);
        if (i + 1 < neighbourKeys.size()) {
            neighbourOk = neighbourOk && next != nullptr && next->key == neighbourKeys[i + 1];
        } else {
            neighbourOk = neighbourOk && next == nullptr;
        }
        if (i > 0) {
            neighbourOk = neighbourOk && previous != nullptr && previous->key == neighbourKeys[i - 1];
        } else {
            neighbourOk = neighbourOk && previous == nullptr;
        }
    }
    check(neighbourOk, "successor and predecessor agree with the sorted reference");

    BST rangeTree;
    DynArray<int> rangeOrder;
    for (int i = 1; i <= 100; i++) {
        rangeOrder.push(i);
    }
    Rng rangeRng(4242u);
    rangeRng.shuffle(rangeOrder);
    for (int i = 0; i < rangeOrder.size(); i++) {
        rangeTree.insert(rangeOrder[i]);
    }
    BST::RangeResult rangeFull = rangeTree.rangeQuery(1, 100);
    DynArray<int> fullExpected;
    for (int i = 1; i <= 100; i++) {
        fullExpected.push(i);
    }
    check(sameArray(rangeFull.keys, fullExpected), "full range returns every key in order");
    BST::RangeResult rangeMiddle = rangeTree.rangeQuery(40, 60);
    DynArray<int> middleExpected;
    for (int i = 40; i <= 60; i++) {
        middleExpected.push(i);
    }
    check(sameArray(rangeMiddle.keys, middleExpected), "range query returns the inclusive middle band");
    BST::RangeResult rangeSingle = rangeTree.rangeQuery(55, 55);
    check(rangeSingle.keys.size() == 1 && rangeSingle.keys[0] == 55, "single-key range works");
    BST::RangeResult rangeEmpty = rangeTree.rangeQuery(1000, 2000);
    check(rangeEmpty.keys.size() == 0, "range outside the tree is empty");
    BST::RangeResult rangeNone = rangeTree.rangeQuery(0, 0);
    check(rangeNone.keys.size() == 0, "range below the tree is empty");
    BST::RangeResult rangeInverted = rangeTree.rangeQuery(60, 40);
    check(rangeInverted.keys.size() == 0 && rangeInverted.events.size() == 1 &&
          rangeInverted.events.at(0).type == EventType::DONE,
          "inverted range is empty and emits only DONE");
    check(countEvents(rangeMiddle.events, EventType::VISIT) > 0, "range query emits VISIT events");
    check(countEvents(rangeMiddle.events, EventType::HIGHLIGHT_SUBTREE) > 0,
          "range query highlights pruned subtrees");
    bool skippedModeOk = true;
    for (int i = 0; i < rangeMiddle.events.size(); i++) {
        if (rangeMiddle.events.at(i).type == EventType::HIGHLIGHT_SUBTREE &&
            rangeMiddle.events.at(i).extra != HighlightMode::SKIPPED) {
            skippedModeOk = false;
        }
    }
    check(skippedModeOk, "pruned subtree highlights use the skipped mode");
    check(rangeMiddle.events.at(rangeMiddle.events.size() - 1).type == EventType::DONE,
          "range query ends with DONE");
    BST emptyRangeTree;
    BST::RangeResult emptyRange = emptyRangeTree.rangeQuery(1, 10);
    check(emptyRange.keys.size() == 0 && emptyRange.events.size() == 1 &&
          emptyRange.events.at(0).type == EventType::DONE,
          "range query on an empty tree emits only DONE");

    BST kthTree;
    DynArray<int> kthOrder;
    for (int i = 1; i <= 100; i++) {
        kthOrder.push(i);
    }
    Rng kthRng(31415u);
    kthRng.shuffle(kthOrder);
    for (int i = 0; i < kthOrder.size(); i++) {
        kthTree.insert(kthOrder[i]);
    }
    DynArray<int> kthReference;
    collectKeys(kthTree, kthReference);
    bool kthOk = true;
    for (int k = 1; k <= 100; k++) {
        BSTNode* node = kthTree.kthSmallest(k);
        kthOk = kthOk && node != nullptr && node->key == kthReference[k - 1];
    }
    check(kthOk, "kthSmallest matches the sorted reference for every k");
    check(kthTree.kthSmallest(0) == nullptr && kthTree.kthSmallest(101) == nullptr,
          "kthSmallest rejects k outside 1..size");
    BST emptyKth;
    check(emptyKth.kthSmallest(1) == nullptr, "kthSmallest on an empty tree is null");

    BST lcaTree;
    for (int i = 0; i < 7; i++) {
        lcaTree.insert(mixedInput[i]);
    }
    check(lcaTree.lcaBST(20, 40) != nullptr && lcaTree.lcaBST(20, 40)->key == 30,
          "lcaBST of siblings is the parent");
    check(lcaTree.lcaBST(20, 60) != nullptr && lcaTree.lcaBST(20, 60)->key == 50,
          "lcaBST across subtrees is the root");
    check(lcaTree.lcaBST(50, 70) != nullptr && lcaTree.lcaBST(50, 70)->key == 50,
          "lcaBST of an ancestor and a descendant is the ancestor");
    check(lcaTree.lcaBST(30, 30) != nullptr && lcaTree.lcaBST(30, 30)->key == 30,
          "lcaBST of the same key is that node");
    check(lcaTree.lcaBST(20, 999) == nullptr, "lcaBST with an absent key is null");
    check(lcaTree.lcaBST(998, 999) == nullptr, "lcaBST with two absent keys is null");

    BST chainZero;
    chainZero.insert(5);
    chainZero.buildChain(0);
    check(chainZero.empty() && chainZero.size() == 0 && chainZero.height() == 0,
          "buildChain(0) clears to an empty tree");
    BST chainOne;
    chainOne.buildChain(1);
    check(chainOne.size() == 1 && chainOne.height() == 1 && chainOne.root() != nullptr &&
          static_cast<BSTNode*>(chainOne.root())->key == 1,
          "buildChain(1) creates the root 1");
    BST chainFive;
    chainFive.buildChain(5);
    BST sequentialFive;
    for (int i = 1; i <= 5; i++) {
        sequentialFive.insert(i);
    }
    check(chainFive.serialize() == sequentialFive.serialize(),
          "buildChain(5) matches five sequential inserts");
    check(chainFive.height() == 5 && chainFive.size() == 5, "buildChain(5) is a five-deep chain");
    check(validateBST(chainFive, why), "checker accepts buildChain(5)");
    BST chainFifty;
    chainFifty.buildChain(50);
    check(chainFifty.height() == 50 && chainFifty.size() == 50, "buildChain(50) is a fifty-deep chain");
    DynArray<int> chainFiftyKeys;
    collectKeys(chainFifty, chainFiftyKeys);
    DynArray<int> chainFiftyExpected;
    for (int i = 1; i <= 50; i++) {
        chainFiftyExpected.push(i);
    }
    check(sameArray(chainFiftyKeys, chainFiftyExpected), "buildChain(50) is ordered 1..50");
    check(validateBST(chainFifty, why), "checker accepts buildChain(50)");
    bool chainIdsOk = true;
    int chainLastId = 0;
    TreeNode* chainNode = chainFifty.root();
    int chainExpectedKey = 1;
    while (chainNode != nullptr) {
        if (chainNode->id <= chainLastId || static_cast<BSTNode*>(chainNode)->key != chainExpectedKey) {
            chainIdsOk = false;
        }
        chainLastId = chainNode->id;
        chainExpectedKey++;
        chainNode = chainNode->right;
    }
    check(chainIdsOk, "buildChain ids are unique and increase along the chain");
    BST chainBig;
    chainBig.buildChain(10000);
    check(chainBig.height() == 10000 && chainBig.size() == 10000,
          "buildChain(10000) builds a 10,000-deep chain");
    bool chainBigOk = true;
    int chainBigLastId = 0;
    TreeNode* chainBigNode = chainBig.root();
    int chainBigKey = 1;
    int chainBigCount = 0;
    while (chainBigNode != nullptr) {
        if (chainBigNode->id <= chainBigLastId ||
            static_cast<BSTNode*>(chainBigNode)->key != chainBigKey) {
            chainBigOk = false;
        }
        chainBigLastId = chainBigNode->id;
        chainBigKey++;
        chainBigCount++;
        chainBigNode = chainBigNode->right;
    }
    check(chainBigOk && chainBigCount == 10000, "10,000-node chain has unique increasing ids and keys");
    check(validateBST(chainBig, why), "checker accepts buildChain(10000)");
    chainBig.clear();
    check(chainBig.empty(), "10,000-node chain is destroyed");

    BST big(false);
    bool bigQuiet = true;
    clock_t start = clock();
    for (int i = 1; i <= 10000; i++) {
        BST::OpResult result = big.insert(i);
        bigQuiet = bigQuiet && result.ok && result.events.empty();
    }
    clock_t finish = clock();
    double elapsed = static_cast<double>(finish - start) / static_cast<double>(CLOCKS_PER_SEC);
    cout << "10,000 sorted inserts (recording off): " << elapsed << " s" << endl;
    check(bigQuiet, "recording off keeps every insert result and an empty log");
    check(big.height() == 10000 && big.size() == 10000,
          "10,000 sorted inserts build a 10,000-deep chain");
    bool bigSpineOk = true;
    int bigLastId = 0;
    TreeNode* bigNode = big.root();
    int bigKey = 1;
    while (bigNode != nullptr) {
        if (bigNode->id <= bigLastId || static_cast<BSTNode*>(bigNode)->key != bigKey) {
            bigSpineOk = false;
        }
        bigLastId = bigNode->id;
        bigKey++;
        bigNode = bigNode->right;
    }
    check(bigSpineOk, "10,000 sorted inserts produce increasing ids and keys");
    big.clear();
    check(big.empty(), "10,000-node insert chain is destroyed");

    BST differential;
    DynArray<int> differentialReference;
    Rng differentialRng(987654u);
    bool differentialOk = true;
    string differentialWhy;
    for (int step = 0; step < 500; step++) {
        int operation = differentialRng.range(0, 2);
        int key = differentialRng.range(1, 300);
        if (operation == 0) {
            bool expectedOk = !referenceContains(differentialReference, key);
            BST::OpResult result = differential.insert(key);
            if (result.ok != expectedOk) {
                differentialOk = false;
                break;
            }
            if (result.ok) {
                referenceInsert(differentialReference, key);
            }
        } else if (operation == 1) {
            bool expectedOk = referenceContains(differentialReference, key);
            BST::OpResult result = differential.deleteKey(key);
            if (result.ok != expectedOk) {
                differentialOk = false;
                break;
            }
            if (result.ok) {
                referenceRemove(differentialReference, key);
            }
        } else {
            bool expectedFound = referenceContains(differentialReference, key);
            BST::SearchResult result = differential.search(key);
            if ((result.found != nullptr) != expectedFound) {
                differentialOk = false;
                break;
            }
        }
        differentialOk = differentialOk && validateBST(differential, differentialWhy);
        DynArray<int> differentialKeys;
        collectKeys(differential, differentialKeys);
        if (!sameArray(differentialKeys, differentialReference)) {
            differentialOk = false;
            break;
        }
    }
    check(differentialOk, "500 seeded differential operations match the sorted reference");
    if (!differentialOk) {
        cout << "  differential checker: " << differentialWhy << endl;
    }
    check(differential.size() == differentialReference.size(),
          "differential tree size matches the reference");

    BST onTree(true);
    BST offTree(false);
    int parityInput[] = {50, 30, 70, 20, 40, 60, 80, 30, 20, 50, 999, 35};
    bool parityOk = true;
    for (int i = 0; i < 12; i++) {
        int key = parityInput[i];
        BST::OpResult onInsert = onTree.insert(key);
        BST::OpResult offInsert = offTree.insert(key);
        parityOk = parityOk && onInsert.ok == offInsert.ok && onInsert.message == offInsert.message;
        parityOk = parityOk && !onInsert.events.empty() && offInsert.events.empty();
        parityOk = parityOk && onTree.serialize() == offTree.serialize();
        parityOk = parityOk && validateBST(onTree, why) && validateBST(offTree, why);
        BST::SearchResult onSearch = onTree.search(key);
        BST::SearchResult offSearch = offTree.search(key);
        parityOk = parityOk && (onSearch.found != nullptr) == (offSearch.found != nullptr);
        parityOk = parityOk && !onSearch.events.empty() && offSearch.events.empty();
        BST::OpResult onDelete = onTree.deleteKey(key);
        BST::OpResult offDelete = offTree.deleteKey(key);
        parityOk = parityOk && onDelete.ok == offDelete.ok && onDelete.message == offDelete.message;
        parityOk = parityOk && !onDelete.events.empty() && offDelete.events.empty();
        parityOk = parityOk && onTree.serialize() == offTree.serialize();
        parityOk = parityOk && validateBST(onTree, why) && validateBST(offTree, why);
        BST::RangeResult onRange = onTree.rangeQuery(10, 60);
        BST::RangeResult offRange = offTree.rangeQuery(10, 60);
        parityOk = parityOk && sameArray(onRange.keys, offRange.keys);
        parityOk = parityOk && !onRange.events.empty() && offRange.events.empty();
    }
    check(parityOk, "recording off matches recording on and produces empty logs");

    BST idTree;
    idTree.insert(10);
    idTree.insert(20);
    idTree.insert(30);
    int idMaximum = 0;
    Stack<TreeNode*> idStack;
    TreeNode* idNode = idTree.root();
    while (idNode != nullptr || !idStack.empty()) {
        while (idNode != nullptr) {
            idStack.push(idNode);
            idNode = idNode->left;
        }
        idNode = idStack.pop();
        if (idNode->id > idMaximum) {
            idMaximum = idNode->id;
        }
        idNode = idNode->right;
    }
    idTree.deleteKey(20);
    idTree.insert(25);
    bool idTreeOk = validateBST(idTree, why);
    BSTNode* idNew = findKey(idTree, 25);
    check(idNew != nullptr && idNew->id > idMaximum, "new ids after a delete are never reused");
    int idAfterDelete = idNew == nullptr ? 0 : idNew->id;
    idTree.clear();
    idTree.insert(5);
    idTreeOk = idTreeOk && validateBST(idTree, why);
    check(idTree.root() != nullptr && idTree.root()->id > idAfterDelete, "ids continue after clear");
    check(idTreeOk, "checker accepts the id tree after every mutation");

    BST inherited;
    bool inheritedValid = true;
    for (int i = 0; i < 7; i++) {
        inherited.insert(mixedInput[i]);
        inheritedValid = inheritedValid && validateBST(inherited, why);
    }
    check(inheritedValid, "checker accepts the inherited BST after every insert");
    BinaryTree::TraversalResult inheritedPre = inherited.preorder();
    check(inheritedPre.ids.size() == 7, "inherited preorder works on a BST");
    BinaryTree::TraversalResult inheritedIn = inherited.inorder();
    DynArray<int> inheritedKeys;
    collectKeys(inherited, inheritedKeys);
    bool inheritedInorderOk = inheritedIn.ids.size() == inheritedKeys.size();
    for (int i = 0; inheritedInorderOk && i < inheritedIn.ids.size(); i++) {
        TreeNode* node = inherited.findById(inheritedIn.ids[i]);
        if (node == nullptr || static_cast<BSTNode*>(node)->key != inheritedKeys[i]) {
            inheritedInorderOk = false;
        }
    }
    check(inheritedInorderOk, "inherited inorder returns ascending keys");
    string inheritedText = inherited.serialize();
    BST inheritedCopy;
    BinaryTree::BuildResult inheritedBuild = inheritedCopy.deserialize(inheritedText);
    check(inheritedBuild.ok && inheritedCopy.serialize() == inheritedText &&
          inheritedCopy.isSameTree(inherited),
          "BST deserializes from its own serialization");
    check(validateBST(inheritedCopy, why), "deserialized BST is valid");

    return failed;
}
