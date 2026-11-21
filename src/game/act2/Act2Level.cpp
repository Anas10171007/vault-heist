#include "game/act2/Act2Level.h"

#include "game/LevelDataBuilder.h"

namespace vh {

Act2Level::Act2Level() : scoring(1) {}

bool Act2Level::load(const LevelDef& definition) {
    definitionData = definition;
    reset();
    return true;
}

void Act2Level::buildTree() {
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

void Act2Level::appendPath(int key, DynArray<int>& path) const {
    TreeNode* current = treeModel.root();
    while (current != nullptr) {
        BSTNode* node = static_cast<BSTNode*>(current);
        path.push(node->id);
        if (key == node->key) break;
        current = key < node->key ? node->left : node->right;
    }
}

void Act2Level::computePlan() {
    const LevelRules& rules = definitionData.rules;
    target = rules.target;
    lo = rules.lo;
    hi = rules.hi;
    expectedCase = DeleteCase::LEAF;
    const std::string& task = definitionData.task;

    if (task == "bst_search" || task == "bst_insert") {
        appendPath(target, expected);
        optimal = expected.size() + (task == "bst_insert" ? 1 : 0);
    } else if (task == "bst_delete") {
        TreeNode* node = treeModel.findByValue(target);
        BSTNode* bst = static_cast<BSTNode*>(node);
        int children = 0;
        if (bst != nullptr) {
            if (bst->left != nullptr) children++;
            if (bst->right != nullptr) children++;
        }
        expectedCase = children == 0 ? DeleteCase::LEAF
                                     : (children == 1 ? DeleteCase::ONE_CHILD : DeleteCase::TWO_CHILD);
        optimal = 2;
    } else if (task == "bst_range") {
        BST::RangeResult result = treeModel.rangeQuery(lo, hi);
        for (int i = 0; i < result.keys.size(); i++) {
            TreeNode* node = treeModel.findByValue(result.keys[i]);
            if (node != nullptr) expected.push(node->id);
        }
        optimal = expected.size();
    } else if (task == "bst_boss") {
        int count = treeModel.size();
        for (int stageIndex = 0; stageIndex < 3; stageIndex++) {
            int depth = count * (stageIndex + 1) / 4;
            if (depth < 1) depth = 1;
            if (depth >= count) depth = count - 1;
            BSTNode* node = treeModel.findMin();
            for (int step = 0; step < depth && node != nullptr; step++) {
                node = treeModel.successor(node->key);
            }
            if (node != nullptr) appendPath(node->key, expected);
            stageEnds.push(expected.size());
        }
        optimal = expected.size();
    }

    if (optimal < 1) optimal = 1;
}

void Act2Level::reset() {
    expected.clear();
    stageEnds.clear();
    progress = 0;
    stage = 0;
    complete = false;
    buildTree();
    computePlan();
    scoring.reset(optimal);
}

PlayerAction Act2Level::nextAction() const {
    const std::string& task = definitionData.task;
    if (task == "bst_insert" && progress >= expected.size()) return confirmAction();
    if (task == "bst_delete") {
        return progress == 0 ? chooseCase(expectedCase) : confirmAction();
    }
    return progress < expected.size() ? clickNode(expected[progress]) : PlayerAction();
}

bool Act2Level::sameAction(const PlayerAction& first, const PlayerAction& second) const {
    if (first.kind != second.kind) return false;
    if (first.kind == ActionKind::CLICK_NODE) return first.id == second.id;
    if (first.kind == ActionKind::CHOOSE_CASE) return first.choice == second.choice;
    return true;
}

PlayResult Act2Level::reject(const char* message) {
    PlayResult result;
    scoring.addPenalty(PenaltyKind::WRONG_TRAVERSAL);
    result.wrong = true;
    result.message = message;
    result.lost = scoring.isFailed();
    if (result.lost) result.message = "alarm capacity reached";
    return result;
}

PlayResult Act2Level::finish(const char* message, EventLog& events) {
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

PlayResult Act2Level::accept(const char* message, EventLog& events) {
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

void Act2Level::advanceBossStage() {
    if (stage + 1 >= stageEnds.size()) return;
    stage++;
    scoring.reset(expected.size() - progress);
}

PlayResult Act2Level::apply(const PlayerAction& action) {
    recordAction(action);
    PlayResult result;
    if (complete || scoring.isFailed()) {
        result.wrong = true;
        result.message = complete ? "level already complete" : "alarm capacity reached";
        result.lost = scoring.isFailed();
        return result;
    }
    if (!sameAction(action, nextAction())) {
        return reject(definitionData.task == "bst_delete" ? "that is not the right delete case"
                                                          : "that is not the next correct move");
    }

    const std::string& task = definitionData.task;
    EventLog events;
    if (task == "bst_delete") {
        if (progress == 0) {
            progress++;
            scoring.addStep();
            result.correct = true;
            result.message = "delete case accepted";
            return result;
        }
        BST::OpResult removed = treeModel.deleteKey(target);
        events.extend(removed.events);
        return accept("target deleted", events);
    }
    if (task == "bst_insert" && action.kind == ActionKind::CONFIRM) {
        BST::OpResult inserted = treeModel.insert(target);
        events.extend(inserted.events);
        return accept("key inserted", events);
    }

    events.append(Event(EventType::VISIT, action.id));
    progress++;
    if (task == "bst_range" && progress >= expected.size()) {
        BST::RangeResult answer = treeModel.rangeQuery(lo, hi);
        int skipped = 0;
        for (int i = 0; i < answer.events.size(); i++) {
            if (answer.events.at(i).type == EventType::HIGHLIGHT_SUBTREE &&
                answer.events.at(i).extra == HighlightMode::SKIPPED) {
                skipped++;
            }
            events.append(answer.events.at(i));
        }
        scoring.addBonus(skipped * 10);
        return accept("range reported with pruning bonus", events);
    }
    if (task == "bst_boss" && progress >= stageEnds[stage]) {
        bool lastStage = stage + 1 >= stageEnds.size();
        scoring.addStep();
        if (scoring.isFailed()) {
            result.correct = true;
            result.lost = true;
            result.message = "alarm capacity reached";
            result.events.extend(events);
            return result;
        }
        if (!lastStage) {
            advanceBossStage();
            result.correct = true;
            result.message = "chain stage cleared, capacity shrinks";
            result.events.extend(events);
            return result;
        }
        return finish("boss chain cleared", events);
    }

    scoring.addStep();
    if (scoring.isFailed()) {
        result.correct = true;
        result.lost = true;
        result.message = "alarm capacity reached";
        result.events.extend(events);
        return result;
    }
    if (progress >= expected.size()) {
        return finish("target found", events);
    }
    result.correct = true;
    result.message = "room searched";
    result.events.extend(events);
    return result;
}

PlayResult Act2Level::hint() {
    recordHint();
    PlayResult result;
    scoring.addPenalty(PenaltyKind::HINT);
    result.hasHint = true;
    result.hintAction = nextAction();
    result.message = "hint used: three stars are no longer available";
    return result;
}

LevelSnapshot Act2Level::captureSnapshot() const {
    LevelSnapshot snapshot = LevelPlay::captureSnapshot();
    snapshot.state.push(progress);
    snapshot.state.push(stage);
    snapshot.state.push(complete ? 1 : 0);
    return snapshot;
}

void Act2Level::restoreSnapshot(const LevelSnapshot& snapshot) {
    LevelPlay::restoreSnapshot(snapshot);
    if (snapshot.state.size() < 3) return;
    progress = snapshot.state[0];
    stage = snapshot.state[1];
    complete = snapshot.state[2] != 0;
}

void Act2Level::restoreModelTree(const TreeSnapshot& snapshot) {
    treeModel.restoreTree(snapshot);
}

void Act2Level::restoreScore(const ScoreSnapshot& snapshot) {
    scoring.restoreState(snapshot);
}

int Act2Level::optimalSteps() const { return optimal; }
bool Act2Level::isComplete() const { return complete; }
bool Act2Level::isFailed() const { return scoring.isFailed(); }
const ScoreSystem& Act2Level::score() const { return scoring; }
const BinaryTree& Act2Level::tree() const { return treeModel; }
const LevelDef& Act2Level::definition() const { return definitionData; }

}
