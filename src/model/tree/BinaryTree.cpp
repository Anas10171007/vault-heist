#include "model/tree/BinaryTree.h"

#include <string>

#include "util/Queue.h"
#include "util/Stack.h"

namespace vh {

namespace {
struct NodeDepth {
    const TreeNode* node;
    int depth;
};
}

BinaryTree::BinaryTree(bool recordEvents)
    : rootNode(nullptr), idSource(), recording(recordEvents) {}

BinaryTree::~BinaryTree() {
    clear();
}

BinaryTree::BinaryTree(BinaryTree&& other) noexcept
    : rootNode(other.rootNode), idSource(other.idSource), recording(other.recording) {
    other.rootNode = nullptr;
}

BinaryTree& BinaryTree::operator=(BinaryTree&& other) noexcept {
    if (this != &other) {
        clear();
        rootNode = other.rootNode;
        idSource = other.idSource;
        recording = other.recording;
        other.rootNode = nullptr;
    }
    return *this;
}

void BinaryTree::clear() {
    destroyNodes(rootNode);
    rootNode = nullptr;
}

bool BinaryTree::empty() const {
    return rootNode == nullptr;
}

int BinaryTree::size() const {
    if (rootNode == nullptr) {
        return 0;
    }
    int count = 0;
    Stack<TreeNode*> pending;
    pending.push(rootNode);
    while (!pending.empty()) {
        TreeNode* current = pending.pop();
        count++;
        if (current->left != nullptr) {
            pending.push(current->left);
        }
        if (current->right != nullptr) {
            pending.push(current->right);
        }
    }
    return count;
}

TreeNode* BinaryTree::root() const {
    return rootNode;
}

void BinaryTree::setRecordEvents(bool enabled) {
    recording = enabled;
}

bool BinaryTree::isRecording() const {
    return recording;
}

BinaryTree::TraversalResult BinaryTree::preorder() const {
    TraversalResult result;
    result.events.setRecordEvents(recording);
    preorderRec(rootNode, result);
    result.events.append(Event(EventType::DONE));
    return result;
}

BinaryTree::TraversalResult BinaryTree::inorder() const {
    TraversalResult result;
    result.events.setRecordEvents(recording);
    inorderRec(rootNode, result);
    result.events.append(Event(EventType::DONE));
    return result;
}

BinaryTree::TraversalResult BinaryTree::postorder() const {
    TraversalResult result;
    result.events.setRecordEvents(recording);
    postorderRec(rootNode, result);
    result.events.append(Event(EventType::DONE));
    return result;
}

BinaryTree::TraversalResult BinaryTree::levelOrder() const {
    TraversalResult result;
    result.events.setRecordEvents(recording);
    int levels = height();
    for (int level = 1; level <= levels; level++) {
        collectLevel(rootNode, level, result);
    }
    result.events.append(Event(EventType::DONE));
    return result;
}

BinaryTree::TraversalResult BinaryTree::preorderIterative() const {
    TraversalResult result;
    result.events.setRecordEvents(recording);
    if (rootNode != nullptr) {
        Stack<TreeNode*> pending;
        pending.push(rootNode);
        while (!pending.empty()) {
            TreeNode* current = pending.pop();
            result.ids.push(current->id);
            result.events.append(Event(EventType::VISIT, current->id));
            if (current->right != nullptr) {
                pending.push(current->right);
            }
            if (current->left != nullptr) {
                pending.push(current->left);
            }
        }
    }
    result.events.append(Event(EventType::DONE));
    return result;
}

BinaryTree::TraversalResult BinaryTree::inorderIterative() const {
    TraversalResult result;
    result.events.setRecordEvents(recording);
    Stack<TreeNode*> pending;
    TreeNode* current = rootNode;
    while (current != nullptr || !pending.empty()) {
        while (current != nullptr) {
            pending.push(current);
            current = current->left;
        }
        current = pending.pop();
        result.ids.push(current->id);
        result.events.append(Event(EventType::VISIT, current->id));
        current = current->right;
    }
    result.events.append(Event(EventType::DONE));
    return result;
}

BinaryTree::TraversalResult BinaryTree::postorderIterative() const {
    TraversalResult result;
    result.events.setRecordEvents(recording);
    if (rootNode != nullptr) {
        Stack<TreeNode*> first;
        Stack<TreeNode*> second;
        first.push(rootNode);
        while (!first.empty()) {
            TreeNode* current = first.pop();
            second.push(current);
            if (current->left != nullptr) {
                first.push(current->left);
            }
            if (current->right != nullptr) {
                first.push(current->right);
            }
        }
        while (!second.empty()) {
            TreeNode* current = second.pop();
            result.ids.push(current->id);
            result.events.append(Event(EventType::VISIT, current->id));
        }
    }
    result.events.append(Event(EventType::DONE));
    return result;
}

BinaryTree::TraversalResult BinaryTree::levelOrderIterative() const {
    TraversalResult result;
    result.events.setRecordEvents(recording);
    if (rootNode != nullptr) {
        Queue<TreeNode*> pending;
        pending.enqueue(rootNode);
        while (!pending.empty()) {
            TreeNode* current = pending.dequeue();
            result.ids.push(current->id);
            result.events.append(Event(EventType::VISIT, current->id));
            if (current->left != nullptr) {
                pending.enqueue(current->left);
            }
            if (current->right != nullptr) {
                pending.enqueue(current->right);
            }
        }
    }
    result.events.append(Event(EventType::DONE));
    return result;
}

BinaryTree::BuildResult BinaryTree::buildFromPreorderInorder(const DynArray<int>& preorder,
                                                             const DynArray<int>& inorder) {
    BuildResult result;
    result.events.setRecordEvents(recording);
    if (!validateReconstructionInputs(preorder, inorder, "preorder", result.error)) {
        return result;
    }
    if (preorder.size() == 0) {
        clear();
        result.ok = true;
        result.events.append(Event(EventType::DONE));
        return result;
    }
    bool ok = true;
    TreeNode* newRoot = buildPreIn(preorder, 0, preorder.size() - 1,
                                   inorder, 0, inorder.size() - 1,
                                   nullptr, Side::NONE, result.events, ok, result.error);
    if (!ok) {
        destroyNodes(newRoot);
        result.events.clear();
        return result;
    }
    clear();
    rootNode = newRoot;
    result.ok = true;
    result.events.append(Event(EventType::DONE));
    return result;
}

BinaryTree::BuildResult BinaryTree::buildFromPostorderInorder(const DynArray<int>& postorder,
                                                              const DynArray<int>& inorder) {
    BuildResult result;
    result.events.setRecordEvents(recording);
    if (!validateReconstructionInputs(postorder, inorder, "postorder", result.error)) {
        return result;
    }
    if (postorder.size() == 0) {
        clear();
        result.ok = true;
        result.events.append(Event(EventType::DONE));
        return result;
    }
    bool ok = true;
    TreeNode* newRoot = buildPostIn(postorder, 0, postorder.size() - 1,
                                    inorder, 0, inorder.size() - 1,
                                    nullptr, Side::NONE, result.events, ok, result.error);
    if (!ok) {
        destroyNodes(newRoot);
        result.events.clear();
        return result;
    }
    clear();
    rootNode = newRoot;
    result.ok = true;
    result.events.append(Event(EventType::DONE));
    return result;
}

int BinaryTree::height() const {
    return height(rootNode);
}

int BinaryTree::height(const TreeNode* node) const {
    if (node == nullptr) {
        return 0;
    }
    int best = 0;
    Stack<NodeDepth> pending;
    NodeDepth start;
    start.node = node;
    start.depth = 1;
    pending.push(start);
    while (!pending.empty()) {
        NodeDepth current = pending.pop();
        if (current.depth > best) {
            best = current.depth;
        }
        if (current.node->left != nullptr) {
            NodeDepth next;
            next.node = current.node->left;
            next.depth = current.depth + 1;
            pending.push(next);
        }
        if (current.node->right != nullptr) {
            NodeDepth next;
            next.node = current.node->right;
            next.depth = current.depth + 1;
            pending.push(next);
        }
    }
    return best;
}

int BinaryTree::diameter() const {
    int best = 0;
    diameterRec(rootNode, best);
    return best;
}

TreeNode* BinaryTree::lca(int firstId, int secondId) const {
    TreeNode* first = findById(firstId);
    TreeNode* second = findById(secondId);
    if (first == nullptr || second == nullptr) {
        return nullptr;
    }
    int firstDepth = depthOf(first);
    int secondDepth = depthOf(second);
    while (firstDepth > secondDepth) {
        first = first->parent;
        firstDepth--;
    }
    while (secondDepth > firstDepth) {
        second = second->parent;
        secondDepth--;
    }
    while (first != second) {
        first = first->parent;
        second = second->parent;
    }
    return first;
}

DynArray<int> BinaryTree::pathTo(int nodeId) const {
    DynArray<int> path;
    TreeNode* node = findById(nodeId);
    if (node == nullptr) {
        return path;
    }
    Stack<int> reversed;
    while (node != nullptr) {
        reversed.push(node->id);
        node = node->parent;
    }
    while (!reversed.empty()) {
        path.push(reversed.pop());
    }
    return path;
}

EventLog BinaryTree::mirror() {
    EventLog events(recording);
    mirrorRec(rootNode);
    events.append(Event(EventType::DONE));
    return events;
}

bool BinaryTree::isSameTree(const BinaryTree& other) const {
    return sameNodes(rootNode, other.rootNode);
}

bool BinaryTree::isSymmetric() const {
    if (rootNode == nullptr) {
        return true;
    }
    return mirrorNodes(rootNode->left, rootNode->right);
}

std::string BinaryTree::serialize() const {
    std::string text;
    serializeRec(rootNode, text);
    if (!text.empty() && text.back() == ' ') {
        text.pop_back();
    }
    return text;
}

BinaryTree::BuildResult BinaryTree::deserialize(const std::string& text) {
    BuildResult result;
    result.events.setRecordEvents(recording);
    int position = 0;
    TreeNode* newRoot = nullptr;
    if (!parseNode(text, position, nullptr, Side::NONE, newRoot, result.events, result.error)) {
        destroyNodes(newRoot);
        result.events.clear();
        return result;
    }
    skipWhitespace(text, position);
    if (position < static_cast<int>(text.size())) {
        result.error = "unexpected trailing token in serialized tree";
        destroyNodes(newRoot);
        result.events.clear();
        return result;
    }
    clear();
    rootNode = newRoot;
    result.ok = true;
    result.events.append(Event(EventType::DONE));
    return result;
}

TreeNode* BinaryTree::findById(int nodeId) const {
    if (rootNode == nullptr) {
        return nullptr;
    }
    Stack<TreeNode*> pending;
    pending.push(rootNode);
    while (!pending.empty()) {
        TreeNode* current = pending.pop();
        if (current->id == nodeId) {
            return current;
        }
        if (current->right != nullptr) {
            pending.push(current->right);
        }
        if (current->left != nullptr) {
            pending.push(current->left);
        }
    }
    return nullptr;
}

TreeNode* BinaryTree::findByValue(int value) const {
    if (rootNode == nullptr) {
        return nullptr;
    }
    Stack<TreeNode*> pending;
    pending.push(rootNode);
    while (!pending.empty()) {
        TreeNode* current = pending.pop();
        if (current->value == value) {
            return current;
        }
        if (current->right != nullptr) {
            pending.push(current->right);
        }
        if (current->left != nullptr) {
            pending.push(current->left);
        }
    }
    return nullptr;
}

bool BinaryTree::swapChildren(int nodeId) {
    TreeNode* node = findById(nodeId);
    if (node == nullptr) {
        return false;
    }
    TreeNode* temporary = node->left;
    node->left = node->right;
    node->right = temporary;
    if (node->left != nullptr) {
        node->left->parent = node;
    }
    if (node->right != nullptr) {
        node->right->parent = node;
    }
    return true;
}

TreeNode* BinaryTree::createNode(int value) {
    return new TreeNode(idSource.next(), value);
}

void BinaryTree::destroyNodes(TreeNode* node) {
    if (node == nullptr) {
        return;
    }
    Queue<TreeNode*> pending;
    pending.enqueue(node);
    while (!pending.empty()) {
        TreeNode* current = pending.dequeue();
        if (current->left != nullptr) {
            pending.enqueue(current->left);
        }
        if (current->right != nullptr) {
            pending.enqueue(current->right);
        }
        delete current;
    }
}

bool BinaryTree::sameNodes(const TreeNode* first, const TreeNode* second) {
    if (first == nullptr && second == nullptr) {
        return true;
    }
    if (first == nullptr || second == nullptr) {
        return false;
    }
    if (first->value != second->value) {
        return false;
    }
    return sameNodes(first->left, second->left) && sameNodes(first->right, second->right);
}

bool BinaryTree::mirrorNodes(const TreeNode* first, const TreeNode* second) {
    if (first == nullptr && second == nullptr) {
        return true;
    }
    if (first == nullptr || second == nullptr) {
        return false;
    }
    if (first->value != second->value) {
        return false;
    }
    return mirrorNodes(first->left, second->right) && mirrorNodes(first->right, second->left);
}

void BinaryTree::skipWhitespace(const std::string& text, int& position) {
    while (position < static_cast<int>(text.size())) {
        char c = text[position];
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
            position++;
        } else {
            break;
        }
    }
}

bool BinaryTree::parseInteger(const std::string& token, int& value) {
    if (token.empty()) {
        return false;
    }
    int index = 0;
    bool negative = false;
    if (token[index] == '-' || token[index] == '+') {
        negative = token[index] == '-';
        index++;
    }
    if (index >= static_cast<int>(token.size())) {
        return false;
    }
    long long magnitude = 0;
    for (; index < static_cast<int>(token.size()); index++) {
        char c = token[index];
        if (c < '0' || c > '9') {
            return false;
        }
        magnitude = magnitude * 10 + (c - '0');
        if (magnitude > 2147483648LL) {
            return false;
        }
    }
    long long signedValue = negative ? -magnitude : magnitude;
    if (signedValue < -2147483648LL || signedValue > 2147483647LL) {
        return false;
    }
    value = static_cast<int>(signedValue);
    return true;
}

void BinaryTree::preorderRec(const TreeNode* node, TraversalResult& result) const {
    if (node == nullptr) {
        return;
    }
    result.ids.push(node->id);
    result.events.append(Event(EventType::VISIT, node->id));
    preorderRec(node->left, result);
    preorderRec(node->right, result);
}

void BinaryTree::inorderRec(const TreeNode* node, TraversalResult& result) const {
    if (node == nullptr) {
        return;
    }
    inorderRec(node->left, result);
    result.ids.push(node->id);
    result.events.append(Event(EventType::VISIT, node->id));
    inorderRec(node->right, result);
}

void BinaryTree::postorderRec(const TreeNode* node, TraversalResult& result) const {
    if (node == nullptr) {
        return;
    }
    postorderRec(node->left, result);
    postorderRec(node->right, result);
    result.ids.push(node->id);
    result.events.append(Event(EventType::VISIT, node->id));
}

void BinaryTree::collectLevel(const TreeNode* node, int level, TraversalResult& result) const {
    if (node == nullptr) {
        return;
    }
    if (level == 1) {
        result.ids.push(node->id);
        result.events.append(Event(EventType::VISIT, node->id));
    } else {
        collectLevel(node->left, level - 1, result);
        collectLevel(node->right, level - 1, result);
    }
}

void BinaryTree::mirrorRec(TreeNode* node) {
    if (node == nullptr) {
        return;
    }
    TreeNode* temp = node->left;
    node->left = node->right;
    node->right = temp;
    mirrorRec(node->left);
    mirrorRec(node->right);
}

int BinaryTree::diameterRec(const TreeNode* node, int& best) const {
    if (node == nullptr) {
        return 0;
    }
    int leftHeight = diameterRec(node->left, best);
    int rightHeight = diameterRec(node->right, best);
    int through = leftHeight + rightHeight;
    if (through > best) {
        best = through;
    }
    return 1 + (leftHeight > rightHeight ? leftHeight : rightHeight);
}

int BinaryTree::depthOf(const TreeNode* node) const {
    int depth = 0;
    while (node->parent != nullptr) {
        node = node->parent;
        depth++;
    }
    return depth;
}

bool BinaryTree::validateReconstructionInputs(const DynArray<int>& first, const DynArray<int>& second,
                                              const char* firstName, std::string& error) const {
    if (first.size() != second.size()) {
        error = std::string(firstName) + " and inorder sizes differ";
        return false;
    }
    for (int i = 0; i < first.size(); i++) {
        for (int j = 0; j < i; j++) {
            if (first[i] == first[j]) {
                error = std::string(firstName) + " contains duplicate value " + std::to_string(first[i]);
                return false;
            }
        }
    }
    for (int i = 0; i < second.size(); i++) {
        for (int j = 0; j < i; j++) {
            if (second[i] == second[j]) {
                error = "inorder contains duplicate value " + std::to_string(second[i]);
                return false;
            }
        }
    }
    for (int i = 0; i < first.size(); i++) {
        bool found = false;
        for (int j = 0; j < second.size(); j++) {
            if (first[i] == second[j]) {
                found = true;
                break;
            }
        }
        if (!found) {
            error = std::string(firstName) + " value " + std::to_string(first[i]) + " is missing from inorder";
            return false;
        }
    }
    return true;
}

TreeNode* BinaryTree::buildPreIn(const DynArray<int>& preorder, int preStart, int preEnd,
                                 const DynArray<int>& inorder, int inStart, int inEnd,
                                 TreeNode* parent, int side, EventLog& events, bool& ok, std::string& error) {
    if (preStart > preEnd) {
        return nullptr;
    }
    int rootValue = preorder[preStart];
    int rootIndex = -1;
    for (int i = inStart; i <= inEnd; i++) {
        if (inorder[i] == rootValue) {
            rootIndex = i;
            break;
        }
    }
    if (rootIndex < 0) {
        ok = false;
        error = "preorder value " + std::to_string(rootValue) + " is missing from inorder";
        return nullptr;
    }
    int leftCount = rootIndex - inStart;
    TreeNode* node = createNode(rootValue);
    node->parent = parent;
    events.append(Event(EventType::INSERT, node->id, parent == nullptr ? 0 : parent->id, side, rootValue));
    node->left = buildPreIn(preorder, preStart + 1, preStart + leftCount,
                            inorder, inStart, rootIndex - 1,
                            node, Side::LEFT, events, ok, error);
    if (ok) {
        node->right = buildPreIn(preorder, preStart + leftCount + 1, preEnd,
                                 inorder, rootIndex + 1, inEnd,
                                 node, Side::RIGHT, events, ok, error);
    }
    return node;
}

TreeNode* BinaryTree::buildPostIn(const DynArray<int>& postorder, int postStart, int postEnd,
                                  const DynArray<int>& inorder, int inStart, int inEnd,
                                  TreeNode* parent, int side, EventLog& events, bool& ok, std::string& error) {
    if (postStart > postEnd) {
        return nullptr;
    }
    int rootValue = postorder[postEnd];
    int rootIndex = -1;
    for (int i = inStart; i <= inEnd; i++) {
        if (inorder[i] == rootValue) {
            rootIndex = i;
            break;
        }
    }
    if (rootIndex < 0) {
        ok = false;
        error = "postorder value " + std::to_string(rootValue) + " is missing from inorder";
        return nullptr;
    }
    int leftCount = rootIndex - inStart;
    TreeNode* node = createNode(rootValue);
    node->parent = parent;
    events.append(Event(EventType::INSERT, node->id, parent == nullptr ? 0 : parent->id, side, rootValue));
    node->left = buildPostIn(postorder, postStart, postStart + leftCount - 1,
                             inorder, inStart, rootIndex - 1,
                             node, Side::LEFT, events, ok, error);
    if (ok) {
        node->right = buildPostIn(postorder, postStart + leftCount, postEnd - 1,
                                  inorder, rootIndex + 1, inEnd,
                                  node, Side::RIGHT, events, ok, error);
    }
    return node;
}

void BinaryTree::serializeRec(const TreeNode* node, std::string& text) const {
    if (node == nullptr) {
        text += "# ";
        return;
    }
    text += std::to_string(node->value);
    text += ' ';
    serializeRec(node->left, text);
    serializeRec(node->right, text);
}

bool BinaryTree::parseNode(const std::string& text, int& position, TreeNode* parent, int side,
                           TreeNode*& node, EventLog& events, std::string& error) {
    skipWhitespace(text, position);
    if (position >= static_cast<int>(text.size())) {
        error = "unexpected end of serialized tree";
        node = nullptr;
        return false;
    }
    int start = position;
    while (position < static_cast<int>(text.size()) && text[position] != ' ' &&
           text[position] != '\t' && text[position] != '\n' && text[position] != '\r') {
        position++;
    }
    std::string token = text.substr(static_cast<std::string::size_type>(start),
                                    static_cast<std::string::size_type>(position - start));
    if (token == "#") {
        node = nullptr;
        return true;
    }
    int value = 0;
    if (!parseInteger(token, value)) {
        error = "invalid token '" + token + "' in serialized tree";
        node = nullptr;
        return false;
    }
    TreeNode* created = createNode(value);
    created->parent = parent;
    events.append(Event(EventType::INSERT, created->id, parent == nullptr ? 0 : parent->id, side, value));
    node = created;
    TreeNode* left = nullptr;
    if (!parseNode(text, position, created, Side::LEFT, left, events, error)) {
        created->left = left;
        return false;
    }
    created->left = left;
    TreeNode* right = nullptr;
    if (!parseNode(text, position, created, Side::RIGHT, right, events, error)) {
        created->right = right;
        return false;
    }
    created->right = right;
    return true;
}

}
