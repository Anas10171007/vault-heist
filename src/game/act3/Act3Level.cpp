#include "game/act3/Act3Level.h"

#include "game/LevelDataBuilder.h"
#include "util/Stack.h"

namespace vh {

namespace {

struct RepairFrame {
    TreeNode* node = nullptr;
    int depth = 0;
};

int absoluteOf(int value) {
    return value < 0 ? -value : value;
}

}

Act3Level::Act3Level() : scoring(1) {}

bool Act3Level::load(const LevelDef& definition) {
    definitionData = definition;
    reset();
    return true;
}

void Act3Level::buildInitialTree() {
    treeModel.clear();
    if (definitionData.data.hasTree) {
        treeModel.deserialize(definitionData.data.tree);
        return;
    }
    DynArray<int> values;
    LevelDataBuilder::buildValues(definitionData, values);
    for (int i = 0; i < values.size(); i++) {
        treeModel.insert(values[i]);
    }
}

Act3Level::Repair Act3Level::findRepair(const AVLTree& tree) const {
    Repair best;
    int bestDepth = -1;
    if (tree.root() == nullptr) return best;

    Stack<RepairFrame> frames;
    RepairFrame first;
    first.node = tree.root();
    frames.push(first);
    while (!frames.empty()) {
        RepairFrame frame = frames.pop();
        AVLNode* node = static_cast<AVLNode*>(frame.node);
        int balance = tree.balanceFactor(node);
        if (absoluteOf(balance) > 1 && frame.depth > bestDepth) {
            AVLNode* heavy = static_cast<AVLNode*>(balance > 1 ? node->left : node->right);
            if (heavy != nullptr) {
                best.found = true;
                best.nodeId = node->id;
                bestDepth = frame.depth;
                if (balance > 1) {
                    best.caseType = tree.balanceFactor(heavy) >= 0 ? RotationType::LL
                                                                   : RotationType::LR;
                    best.pivotId = best.caseType == RotationType::LL ? node->id : heavy->id;
                } else {
                    best.caseType = tree.balanceFactor(heavy) <= 0 ? RotationType::RR
                                                                   : RotationType::RL;
                    best.pivotId = best.caseType == RotationType::RR ? node->id : heavy->id;
                }
            }
        }
        if (node->left != nullptr) {
            RepairFrame child;
            child.node = node->left;
            child.depth = frame.depth + 1;
            frames.push(child);
        }
        if (node->right != nullptr) {
            RepairFrame child;
            child.node = node->right;
            child.depth = frame.depth + 1;
            frames.push(child);
        }
    }
    return best;
}

void Act3Level::applyRepair(const Repair& target, EventLog& events) {
    if (!target.found) return;
    BST::OpResult result;
    if (target.caseType == RotationType::LL) {
        result = treeModel.rotateRight(target.nodeId);
        events.extend(result.events);
    } else if (target.caseType == RotationType::RR) {
        result = treeModel.rotateLeft(target.nodeId);
        events.extend(result.events);
    } else if (target.caseType == RotationType::LR) {
        result = treeModel.rotateLeft(target.pivotId);
        events.extend(result.events);
        result = treeModel.rotateRight(target.nodeId);
        events.extend(result.events);
    } else {
        result = treeModel.rotateRight(target.pivotId);
        events.extend(result.events);
        result = treeModel.rotateLeft(target.nodeId);
        events.extend(result.events);
    }
}

int Act3Level::predictedRotation(int key) const {
    AVLTree clone(false);
    clone.deserialize(treeModel.serialize());
    BST::OpResult inserted = clone.insert(key);
    for (int i = 0; i < inserted.events.size(); i++) {
        if (inserted.events.at(i).type == EventType::ROTATE) {
            return inserted.events.at(i).extra;
        }
    }
    return -1;
}

void Act3Level::prepareKthStage() {
    TreeNode* root = treeModel.root();
    if (root == nullptr) return;
    if (stage % 2 == 0) {
        BST::OpResult result = treeModel.rotateRight(root->id);
        (void)result;
    } else {
        BST::OpResult result = treeModel.rotateLeft(root->id);
        (void)result;
    }
    repair = findRepair(treeModel);
}

void Act3Level::computePlan() {
    const std::string& task = definitionData.task;
    expected.clear();
    inserts.clear();

    if (task == "avl_badges") {
        BinaryTree::TraversalResult result = treeModel.preorder();
        for (int i = 0; i < result.ids.size(); i++) expected.push(result.ids[i]);
        optimal = expected.size();
    } else if (task == "avl_rotate_single" || task == "avl_rotate_double") {
        AVLTree clone(false);
        clone.deserialize(treeModel.serialize());
        int count = 0;
        while (!clone.isBalanced()) {
            Repair next = findRepair(clone);
            if (!next.found) break;
            if (next.caseType == RotationType::LL) clone.rotateRight(next.nodeId);
            else if (next.caseType == RotationType::RR) clone.rotateLeft(next.nodeId);
            else if (next.caseType == RotationType::LR) {
                clone.rotateLeft(next.pivotId);
                clone.rotateRight(next.nodeId);
            } else {
                clone.rotateRight(next.pivotId);
                clone.rotateLeft(next.nodeId);
            }
            count++;
        }
        optimal = count;
        repair = findRepair(treeModel);
    } else if (task == "avl_auto_predict") {
        DynArray<int> configured = definitionData.rules.inserts;
        for (int i = 0; i < configured.size(); i++) inserts.push(configured[i]);
        if (inserts.empty()) {
            BSTNode* minimum = treeModel.findMin();
            BSTNode* maximum = treeModel.findMax();
            int low = minimum == nullptr ? 1 : minimum->key - 1;
            int high = maximum == nullptr ? 99 : maximum->key + 1;
            inserts.push(low);
            inserts.push(high);
            inserts.push(high + 1);
        }
        optimal = inserts.size();
    } else if (task == "avl_kth") {
        k = definitionData.rules.k;
        if (k < 1) k = treeModel.size() / 2 + 1;
        stages = k + 1 <= treeModel.size() ? 2 : 1;
        optimal = stages * 2;
    }

    if (optimal < 1) optimal = 1;
}

void Act3Level::reset() {
    progress = 0;
    stage = 0;
    phase = 0;
    insertIndex = 0;
    complete = false;
    buildInitialTree();
    expectedSize = treeModel.size();
    computePlan();
    if (definitionData.task == "avl_kth") prepareKthStage();
    scoring.reset(optimal);
}

PlayerAction Act3Level::nextAction() const {
    const std::string& task = definitionData.task;
    if (task == "avl_badges") {
        return progress < expected.size() ? clickNode(expected[progress]) : PlayerAction();
    }
    if (task == "avl_rotate_single") {
        return repair.found ? clickNode(repair.pivotId) : PlayerAction();
    }
    if (task == "avl_rotate_double") {
        if (!repair.found) return PlayerAction();
        return phase == 0 ? chooseCase(repair.caseType) : clickNode(repair.pivotId);
    }
    if (task == "avl_auto_predict") {
        if (insertIndex >= inserts.size()) return PlayerAction();
        return chooseCase(predictedRotation(inserts[insertIndex]));
    }
    if (task == "avl_kth") {
        if (phase == 0) {
            int position = treeModel.size() - k + 1;
            if (position < 1) position = 1;
            BSTNode* node = treeModel.kth(position);
            return answer(node == nullptr ? 0 : node->key);
        }
        return repair.found ? clickNode(repair.pivotId) : PlayerAction();
    }
    return PlayerAction();
}

bool Act3Level::sameAction(const PlayerAction& first, const PlayerAction& second) const {
    if (first.kind != second.kind) return false;
    if (first.kind == ActionKind::CLICK_NODE) return first.id == second.id;
    if (first.kind == ActionKind::ANSWER) return first.value == second.value;
    if (first.kind == ActionKind::CHOOSE_CASE) return first.choice == second.choice;
    return true;
}

PlayResult Act3Level::reject(const char* message, bool rotation) {
    PlayResult result;
    scoring.addPenalty(rotation ? PenaltyKind::WRONG_ROTATION : PenaltyKind::WRONG_TRAVERSAL);
    result.wrong = true;
    result.message = message;
    result.lost = scoring.isFailed();
    if (result.lost) result.message = "alarm capacity reached";
    return result;
}

PlayResult Act3Level::finish(const char* message, EventLog& events) {
    complete = true;
    scoring.finish();
    events.append(Event(EventType::DONE));
    PlayResult result;
    result.correct = true;
    result.won = true;
    result.message = message;
    result.events.extend(events);
    return result;
}

PlayResult Act3Level::accept(const char* message, EventLog& events) {
    scoring.addStep();
    if (scoring.isFailed()) {
        PlayResult result;
        result.correct = true;
        result.lost = true;
        result.message = "alarm capacity reached";
        result.events.extend(events);
        return result;
    }
    return finish(message, events);
}

PlayResult Act3Level::apply(const PlayerAction& action) {
    PlayResult result;
    if (complete || scoring.isFailed()) {
        result.wrong = true;
        result.message = complete ? "level already complete" : "alarm capacity reached";
        result.lost = scoring.isFailed();
        return result;
    }

    const std::string& task = definitionData.task;
    bool rotationTask = task == "avl_rotate_single" || task == "avl_rotate_double" ||
                        task == "avl_auto_predict" || task == "avl_kth";
    if (!sameAction(action, nextAction())) {
        return reject(task == "avl_badges" || (task == "avl_kth" && phase == 0)
                          ? "that is not the correct badge or answer"
                          : "that is not the correct rotation",
                      rotationTask && !(task == "avl_kth" && phase == 0));
    }

    EventLog events;
    if (task == "avl_badges") {
        events.append(Event(EventType::VISIT, action.id));
        progress++;
        if (progress >= expected.size()) return accept("all badges checked", events);
        scoring.addStep();
        result.correct = true;
        result.message = "badge checked";
        result.events.extend(events);
        return result;
    }

    if (task == "avl_rotate_single") {
        applyRepair(repair, events);
        scoring.addStep();
        if (scoring.isFailed()) {
            result.correct = true;
            result.lost = true;
            result.message = "alarm capacity reached";
            result.events.extend(events);
            return result;
        }
        if (treeModel.isBalanced()) return finish("tree balanced", events);
        repair = findRepair(treeModel);
        result.correct = true;
        result.message = "rotation accepted";
        result.events.extend(events);
        return result;
    }

    if (task == "avl_rotate_double") {
        if (phase == 0) {
            phase = 1;
            result.correct = true;
            result.message = "rotation case named";
            return result;
        }
        applyRepair(repair, events);
        phase = 0;
        scoring.addStep();
        if (scoring.isFailed()) {
            result.correct = true;
            result.lost = true;
            result.message = "alarm capacity reached";
            result.events.extend(events);
            return result;
        }
        if (treeModel.isBalanced()) return finish("double rotation complete", events);
        repair = findRepair(treeModel);
        result.correct = true;
        result.message = "double rotation accepted";
        result.events.extend(events);
        return result;
    }

    if (task == "avl_auto_predict") {
        bool right = predictedRotation(inserts[insertIndex]) == action.choice;
        BST::OpResult inserted = treeModel.insert(inserts[insertIndex]);
        events.extend(inserted.events);
        insertIndex++;
        if (!right) {
            PlayResult bad = reject("wrong rotation prediction", true);
            bad.events.extend(events);
            return bad;
        }
        scoring.addStep();
        if (scoring.isFailed()) {
            result.correct = true;
            result.lost = true;
            result.message = "alarm capacity reached";
            result.events.extend(events);
            return result;
        }
        if (insertIndex >= inserts.size()) return finish("all insertions predicted", events);
        result.correct = true;
        result.message = "prediction accepted";
        result.events.extend(events);
        return result;
    }

    if (phase == 0) {
        phase = 1;
        scoring.addStep();
        if (scoring.isFailed()) {
            result.correct = true;
            result.lost = true;
            result.message = "alarm capacity reached";
            return result;
        }
        result.correct = true;
        result.message = "order statistic accepted";
        return result;
    }

    applyRepair(repair, events);
    phase = 0;
    if (treeModel.size() != expectedSize) {
        return reject("rotation changed the node count", true);
    }
    scoring.addStep();
    if (scoring.isFailed()) {
        result.correct = true;
        result.lost = true;
        result.message = "alarm capacity reached";
        result.events.extend(events);
        return result;
    }
    stage++;
    if (stage >= stages) return finish("order statistics verified", events);
    prepareKthStage();
    result.correct = true;
    result.message = "rotation verified, next order statistic ready";
    result.events.extend(events);
    return result;
}

PlayResult Act3Level::hint() {
    PlayResult result;
    scoring.addPenalty(PenaltyKind::HINT);
    result.hasHint = true;
    result.hintAction = nextAction();
    result.message = "hint used: three stars are no longer available";
    return result;
}

int Act3Level::optimalSteps() const { return optimal; }
bool Act3Level::isComplete() const { return complete; }
bool Act3Level::isFailed() const { return scoring.isFailed(); }
const ScoreSystem& Act3Level::score() const { return scoring; }
const BinaryTree& Act3Level::tree() const { return treeModel; }
const LevelDef& Act3Level::definition() const { return definitionData; }

}
