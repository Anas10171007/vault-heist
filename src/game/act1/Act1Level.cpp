#include "game/act1/Act1Level.h"

#include "game/LevelDataBuilder.h"
#include "model/tree/BST.h"

namespace vh {

Act1Level::Act1Level() : scoring(1) {}

bool Act1Level::load(const LevelDef& definition) {
    definitionData = definition;
    reset();
    return true;
}

void Act1Level::reset() {
    treeModel.clear();
    targetTree.clear();
    expected.clear();
    placed.clear();
    progress = 0;
    stage = 0;
    complete = false;
    firstValue = 0;
    secondValue = 0;
    answerOne = 0;
    answerTwo = 0;

    if (definitionData.data.hasTree) {
        treeModel.deserialize(definitionData.data.tree);
    } else {
        DynArray<int> values;
        LevelDataBuilder::buildValues(definitionData, values);
        BST builder;
        for (int i = 0; i < values.size(); i++) {
            builder.insert(values[i]);
        }
        treeModel.deserialize(builder.serialize());
    }

    const std::string& task = definitionData.task;
    if (task == "traversal_click") {
        const std::string& order = definitionData.rules.traversal;
        BinaryTree::TraversalResult result;
        if (order == "inorder") result = treeModel.inorder();
        else if (order == "postorder") result = treeModel.postorder();
        else if (order == "level") result = treeModel.levelOrder();
        else result = treeModel.preorder();
        for (int i = 0; i < result.ids.size(); i++) expected.push(result.ids[i]);
    } else if (task == "reconstruct") {
        targetTree.deserialize(treeModel.serialize());
        BinaryTree::TraversalResult result = definitionData.rules.from == "post_in"
                                                 ? targetTree.postorder()
                                                 : targetTree.preorder();
        for (int i = 0; i < result.ids.size(); i++) {
            TreeNode* node = targetTree.findById(result.ids[i]);
            expected.push(node == nullptr ? 0 : node->value);
            placed.push(0);
        }
    } else if (task == "mirror") {
        targetTree.deserialize(treeModel.serialize());
        targetTree.mirror();
        BinaryTree::TraversalResult result = treeModel.preorder();
        for (int i = 0; i < result.ids.size(); i++) {
            TreeNode* node = treeModel.findById(result.ids[i]);
            if (node != nullptr && (node->left != nullptr || node->right != nullptr)) {
                expected.push(node->id);
            }
        }
    } else if (task == "height_diameter") {
        answerOne = treeModel.height();
        answerTwo = treeModel.diameter();
    } else if (task == "lca_path") {
        BinaryTree::TraversalResult result = treeModel.inorder();
        DynArray<int> values;
        for (int i = 0; i < result.ids.size(); i++) {
            TreeNode* node = treeModel.findById(result.ids[i]);
            if (node != nullptr) values.push(node->value);
        }
        firstValue = definitionData.rules.a;
        secondValue = definitionData.rules.b;
        if (values.size() > 0) {
            if (firstValue == 0) firstValue = values[0];
            if (secondValue == 0) secondValue = values[values.size() - 1];
        }
        TreeNode* first = treeModel.findByValue(firstValue);
        TreeNode* second = treeModel.findByValue(secondValue);
        TreeNode* common = treeModel.lca(first == nullptr ? -1 : first->id,
                                    second == nullptr ? -1 : second->id);
        answerOne = common == nullptr ? 0 : common->value;
        int commonDepth = common == nullptr ? 0 : treeModel.pathTo(common->id).size() - 1;
        int firstDepth = first == nullptr ? 0 : treeModel.pathTo(first->id).size() - 1;
        int secondDepth = second == nullptr ? 0 : treeModel.pathTo(second->id).size() - 1;
        answerTwo = firstDepth + secondDepth - 2 * commonDepth;
    }

    optimal = expected.size() > 0 ? expected.size() : 2;
    if (definitionData.task == "height_diameter" || definitionData.task == "lca_path") {
        optimal = 2;
    }
    if (optimal < 1) optimal = 1;
    scoring.reset(optimal);
}

PlayerAction Act1Level::nextAction() const {
    const std::string& task = definitionData.task;
    if (task == "traversal_click") {
        return progress < expected.size() ? clickNode(expected[progress]) : PlayerAction();
    }
    if (task == "reconstruct") {
        return progress < expected.size() ? placeToken(progress, expected[progress]) : PlayerAction();
    }
    if (task == "mirror") {
        return progress < expected.size() ? clickNode(expected[progress]) : PlayerAction();
    }
    if (task == "height_diameter") {
        return answer(stage == 0 ? answerOne : answerTwo);
    }
    if (task == "lca_path") {
        return answer(stage == 0 ? answerOne : answerTwo);
    }
    return PlayerAction();
}

bool Act1Level::sameAction(const PlayerAction& first, const PlayerAction& second) const {
    if (first.kind != second.kind) return false;
    if (first.kind == ActionKind::CLICK_NODE) return first.id == second.id;
    if (first.kind == ActionKind::PLACE_TOKEN) {
        return first.slot == second.slot && first.value == second.value;
    }
    if (first.kind == ActionKind::ANSWER) return first.value == second.value;
    if (first.kind == ActionKind::CHOOSE_CASE) return first.choice == second.choice;
    return true;
}

PlayResult Act1Level::reject(const char* message) {
    PlayResult result;
    scoring.addPenalty(PenaltyKind::WRONG_TRAVERSAL);
    result.wrong = true;
    result.message = message;
    result.lost = scoring.isFailed();
    if (result.lost) result.message = "alarm capacity reached";
    return result;
}

PlayResult Act1Level::finish(const char* message, EventLog& events) {
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

PlayResult Act1Level::accept(const char* message, EventLog& events) {
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

bool Act1Level::placementCompletesTree() const {
    if (placed.size() != expected.size()) return false;
    for (int i = 0; i < placed.size(); i++) {
        if (placed[i] == 0) return false;
    }

    DynArray<int> ordered;
    DynArray<int> inorder;
    bool post = definitionData.rules.from == "post_in";
    for (int i = 0; i < placed.size(); i++) ordered.push(placed[i]);
    BinaryTree::TraversalResult result = targetTree.inorder();
    for (int i = 0; i < result.ids.size(); i++) {
        TreeNode* node = targetTree.findById(result.ids[i]);
        if (node != nullptr) inorder.push(node->value);
    }

    BinaryTree candidate;
    BinaryTree::BuildResult built = post
                                        ? candidate.buildFromPostorderInorder(ordered, inorder)
                                        : candidate.buildFromPreorderInorder(ordered, inorder);
    return built.ok && candidate.isSameTree(targetTree);
}

PlayResult Act1Level::apply(const PlayerAction& action) {
    PlayResult result;
    if (complete || scoring.isFailed()) {
        result.wrong = true;
        result.message = complete ? "level already complete" : "alarm capacity reached";
        result.lost = scoring.isFailed();
        return result;
    }

    const std::string& task = definitionData.task;
    if (task == "mirror" && action.kind == ActionKind::AUTO) {
        EventLog events;
        while (progress < expected.size()) {
            treeModel.swapChildren(expected[progress]);
            events.append(Event(EventType::HIGHLIGHT_SUBTREE, expected[progress], 0, Side::NONE, 0, 0, 0,
                                HighlightMode::RESULT));
            progress++;
        }
        scoring.addPenalty(PenaltyKind::HINT);
        scoring.addStep();
        if (scoring.isFailed()) {
            result.correct = true;
            result.lost = true;
            result.message = "alarm capacity reached";
            result.events.extend(events);
            return result;
        }
        return finish("mirror completed with auto", events);
    }

    if (!sameAction(action, nextAction())) {
        return reject(task == "reconstruct" ? "that token does not belong in this slot"
                                            : "that is not the next correct move");
    }

    EventLog events;
    if (task == "traversal_click") {
        events.append(Event(EventType::VISIT, action.id));
        progress++;
        return accept("room visited in the right order", events);
    }
    if (task == "reconstruct") {
        placed[action.slot] = action.value;
        events.append(Event(EventType::INSERT, action.slot + 1, 0, Side::NONE, action.value));
        progress++;
        if (progress < placed.size()) {
            scoring.addStep();
            result.correct = true;
            result.message = "token placed";
            result.events.extend(events);
            return result;
        }
        if (!placementCompletesTree()) {
            return reject("the tokens do not rebuild the target tree");
        }
        return accept("blueprint reconstructed", events);
    }
    if (task == "mirror") {
        treeModel.swapChildren(action.id);
        events.append(Event(EventType::HIGHLIGHT_SUBTREE, action.id, 0, Side::NONE, 0, 0, 0,
                            HighlightMode::RESULT));
        progress++;
        if (!treeModel.isSameTree(targetTree)) {
            scoring.addStep();
            result.correct = true;
            result.message = "children swapped";
            result.events.extend(events);
            return result;
        }
        return accept("tree mirrored", events);
    }

    stage++;
    TreeNode* root = treeModel.root();
    events.append(Event(EventType::HIGHLIGHT_SUBTREE, root == nullptr ? 0 : root->id,
                        action.value, Side::NONE, action.value, 0, 0, HighlightMode::RESULT));
    if (stage < 2) {
        scoring.addStep();
        result.correct = true;
        result.message = "answer accepted";
        result.events.extend(events);
        return result;
    }
    return accept("question answered", events);
}

PlayResult Act1Level::hint() {
    PlayResult result;
    scoring.addPenalty(PenaltyKind::HINT);
    result.hasHint = true;
    result.hintAction = nextAction();
    result.message = "hint used: three stars are no longer available";
    return result;
}

int Act1Level::optimalSteps() const { return optimal; }
bool Act1Level::isComplete() const { return complete; }
bool Act1Level::isFailed() const { return scoring.isFailed(); }
const ScoreSystem& Act1Level::score() const { return scoring; }
const BinaryTree& Act1Level::tree() const { return treeModel; }
const LevelDef& Act1Level::definition() const { return definitionData; }

}
