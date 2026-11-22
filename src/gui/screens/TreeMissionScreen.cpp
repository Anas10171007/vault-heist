#include "gui/screens/TreeMissionScreen.h"

#include "game/LevelDataBuilder.h"
#include "game/LevelPlayFactory.h"
#include "gui/core/Renderer.h"
#include "gui/core/ScreenId.h"
#include "model/Node.h"
#include "model/tree/BinaryTree.h"

using namespace std;

namespace vh {

namespace {

bool hit(sf::FloatRect bounds, sf::Vector2f point) {
    return bounds.contains(point);
}

sf::Vector2f uiPoint(const sf::RenderWindow& window, sf::Vector2i pixel) {
    return window.mapPixelToCoords(pixel);
}

}

sf::FloatRect TreeMissionScreen::playfield() {
    return {{40.f, 118.f}, {716.f, 368.f}};
}

sf::FloatRect TreeMissionScreen::tokenRect(int index) {
    return {{40.f + (index % 12) * 58.f, 518.f}, {54.f, 30.f}};
}

sf::FloatRect TreeMissionScreen::slotRect(int index) {
    return {{40.f + (index % 14) * 50.f, 562.f}, {46.f, 30.f}};
}

sf::FloatRect TreeMissionScreen::caseRect(int index, float width) {
    return {{40.f + index * (width + 8.f), 530.f}, {width, 40.f}};
}

sf::FloatRect TreeMissionScreen::submitRect() {
    return {{540.f, 530.f}, {200.f, 40.f}};
}

TreeMissionScreen::TreeMissionScreen(Theme& sharedTheme) : theme(sharedTheme), center(0.f, 0.f) {
    const char* paths[] = {"data/levels", "../data/levels", "../../data/levels"};
    for (const char* path : paths) {
        if (manager.loadAll(path)) break;
    }
    animation.setListener(&bridge);
    treeView.setOrigin(0.f, 0.f);
    treeView.setShowEventLabel(false);
    screenTarget = ScreenId::NONE;
}

TreeMissionScreen::~TreeMissionScreen() {
    delete logic;
}

bool TreeMissionScreen::loadLevel(const string& id) {
    if (manager.size() == 0) {
        const char* paths[] = {"data/levels", "../data/levels", "../../data/levels"};
        for (const char* path : paths) {
            if (manager.loadAll(path)) break;
        }
    }
    const LevelDef* definition = manager.load(id);
    if (definition == nullptr) return false;
    LevelPlay* next = createLevelPlay(*definition);
    if (next == nullptr || !next->load(*definition)) {
        delete next;
        return false;
    }
    delete logic;
    logic = next;
    logic->restart();
    levelId = id;
    undoStack.clear();
    undoUi.clear();
    marks.clear();
    answer = 0;
    selectedNode = 0;
    wrongNode = 0;
    pendingResult = false;
    screenTarget = ScreenId::NONE;
    clearPan();
    refreshTokens();
    message = objectiveFor(*definition);
    rebuildStill();
    return true;
}

void TreeMissionScreen::refreshTokens() {
    tokens.clear();
    slotValue.clear();
    slotFilled.clear();
    selectedToken = -1;
    selectedSlot = -1;
    if (logic == nullptr) return;
    const LevelData& data = logic->definition().data;
    if (data.hasValues) {
        for (int i = 0; i < data.values.size(); i++) tokens.push(data.values[i]);
    } else if (data.hasTree) {
        BinaryTree source;
        source.deserialize(data.tree);
        BinaryTree::TraversalResult result = source.preorder();
        for (int i = 0; i < result.ids.size(); i++) {
            TreeNode* node = source.findById(result.ids[i]);
            if (node != nullptr) tokens.push(node->value);
        }
    } else {
        DynArray<int> values;
        LevelDataBuilder::buildValues(logic->definition(), values);
        for (int i = 0; i < values.size(); i++) tokens.push(values[i]);
    }
    for (int i = 0; i < tokens.size(); i++) {
        slotValue.push(0);
        slotFilled.push(0);
    }
}

void TreeMissionScreen::rebuildStill() {
    if (logic == nullptr) return;
    treeView.setModel(logic->tree());
    TreeLayout layout = TreeLayout::compute(logic->tree());
    EventLog none;
    treeView.setMotion(TreeMotion::simple(layout, none, layout));
    animation.load(none);
    fitLayout(layout);
}

void TreeMissionScreen::fitBox(float minX, float minY, float maxX, float maxY) {
    float margin = 52.f;
    float width = (maxX - minX) + margin * 2.f;
    float height = (maxY - minY) + margin * 2.f;
    if (width < 1.f) width = 1.f;
    if (height < 1.f) height = 1.f;
    sf::FloatRect field = playfield();
    float needX = width / field.size.x;
    float needY = height / field.size.y;
    scale = needX > needY ? needX : needY;
    if (scale < 0.05f) scale = 0.05f;
    if (scale > 12.f) scale = 12.f;
    center = {(minX + maxX) / 2.f, (minY + maxY) / 2.f};
}

void TreeMissionScreen::fitLayout(const TreeLayout& layout) {
    if (layout.empty()) {
        center = {0.f, 0.f};
        scale = 1.f;
        if (logic != nullptr && logic->tree().root() == nullptr) {
            message = "Tree failed to load. Reset or reopen the level.";
        }
        return;
    }
    LayoutBox box = layout.bounds();
    fitBox(box.minX, box.minY, box.maxX, box.maxY);
}

void TreeMissionScreen::fitUnion(const TreeLayout& before, const TreeLayout& after) {
    if (before.empty()) {
        fitLayout(after);
        return;
    }
    if (after.empty()) {
        fitLayout(before);
        return;
    }
    LayoutBox first = before.bounds();
    LayoutBox second = after.bounds();
    float minX = first.minX < second.minX ? first.minX : second.minX;
    float minY = first.minY < second.minY ? first.minY : second.minY;
    float maxX = first.maxX > second.maxX ? first.maxX : second.maxX;
    float maxY = first.maxY > second.maxY ? first.maxY : second.maxY;
    fitBox(minX, minY, maxX, maxY);
}

void TreeMissionScreen::syncView() {
    treeView.reset();
    int count = animation.eventCount();
    if (count == 0 || logic == nullptr) return;
    int shown = animation.completedCount();
    int active = 0;
    float progress = 1.f;
    bool playingNow = animation.isPlaying() && shown < count;
    if (playingNow) {
        active = shown;
        progress = bridge.heard ? bridge.progress : 0.f;
    } else if (shown <= 0) {
        return;
    } else if (shown >= count) {
        active = count - 1;
    } else {
        active = shown - 1;
    }

    const string& task = logic->definition().task;
    for (int i = 0; i <= active; i++) {
        Event shownEvent = animation.eventAt(i);
        if (task == "reconstruct" && shownEvent.type == EventType::INSERT) {
            shownEvent = Event(EventType::DONE);
        }
        float amount = i == active ? progress : 1.f;
        treeView.onEvent(shownEvent, amount);
    }
}

void TreeMissionScreen::remember(const EventLog& events, const TreeLayout& layout) {
    if (logic == nullptr) return;
    const string& task = logic->definition().task;
    for (int i = 0; i < events.size(); i++) {
        const Event& event = events.at(i);
        bool useful = event.type == EventType::VISIT || event.type == EventType::COMPARE ||
                      event.type == EventType::ROTATE || event.type == EventType::HIGHLIGHT_SUBTREE ||
                      event.type == EventType::SWAP_VALUE || event.type == EventType::BALANCE_ALERT ||
                      event.type == EventType::UNLINK ||
                      (event.type == EventType::INSERT && task != "reconstruct");
        if (!useful) continue;
        int ids[2] = {event.idA, event.idB};
        for (int n = 0; n < 2; n++) {
            int id = ids[n];
            if (id == 0 || !layout.contains(id) || marked(id)) continue;
            marks.push(id);
        }
    }
}

void TreeMissionScreen::trimMarks(int count) {
    if (count < 0) count = 0;
    if (count >= marks.size()) return;
    DynArray<int> kept;
    for (int i = 0; i < count; i++) kept.push(marks[i]);
    marks = kept;
}

bool TreeMissionScreen::marked(int nodeId) const {
    for (int i = 0; i < marks.size(); i++) {
        if (marks[i] == nodeId) return true;
    }
    return false;
}

void TreeMissionScreen::pushUndo(const PlayerAction& action) {
    UiUndo ui;
    ui.markCount = marks.size();
    ui.selectedNode = selectedNode;
    ui.selectedToken = selectedToken;
    ui.answer = answer;
    if (action.kind == ActionKind::PLACE_TOKEN && action.slot >= 0 && action.slot < slotFilled.size()) {
        ui.slotIndex = action.slot;
        ui.previousValue = slotValue[action.slot];
        ui.previousFilled = slotFilled[action.slot];
    }
    undoUi.push(ui);
    undoStack.push(logic->captureSnapshot());
}

void TreeMissionScreen::armResult() {
    pendingResult = true;
}

void TreeMissionScreen::apply(const PlayerAction& action) {
    if (logic == nullptr || pendingResult) return;
    TreeLayout beforeLayout = TreeLayout::compute(logic->tree());
    pushUndo(action);
    PlayResult result = logic->apply(action);
    TreeLayout afterLayout = TreeLayout::compute(logic->tree());
    treeView.setModel(logic->tree());
    message = result.message.empty() ? describeAction(action) : result.message;

    EventLog played;
    played.extend(result.events);
    if (!result.correct) {
        if (action.kind == ActionKind::CLICK_NODE) wrongNode = action.id;
        if (result.lost) armResult();
        return;
    }

    wrongNode = 0;
    if (action.kind == ActionKind::PLACE_TOKEN && action.slot >= 0 && action.slot < slotValue.size()) {
        slotValue[action.slot] = action.value;
        slotFilled[action.slot] = 1;
        selectedToken = -1;
        selectedSlot = -1;
    }
    remember(played, afterLayout);
    if (played.size() > 0) {
        treeView.setMotion(TreeMotion::simple(beforeLayout, played, afterLayout));
        animation.load(played);
        animation.play();
        fitUnion(beforeLayout, afterLayout);
    } else {
        treeView.setMotion(TreeMotion::simple(afterLayout, EventLog(), afterLayout));
        animation.load(EventLog());
        fitLayout(afterLayout);
    }
    if (result.won || result.lost) armResult();
}

void TreeMissionScreen::undo() {
    if (logic == nullptr || undoStack.empty() || undoUi.empty()) return;
    logic->restoreSnapshot(undoStack.pop());
    UiUndo ui = undoUi.pop();
    trimMarks(ui.markCount);
    if (ui.slotIndex >= 0 && ui.slotIndex < slotValue.size()) {
        slotValue[ui.slotIndex] = ui.previousValue;
        slotFilled[ui.slotIndex] = ui.previousFilled;
    }
    selectedNode = ui.selectedNode;
    selectedToken = ui.selectedToken;
    answer = ui.answer;
    wrongNode = 0;
    pendingResult = false;
    treeView.setModel(logic->tree());
    rebuildStill();
    message = "Undone";
}

void TreeMissionScreen::resetLevel() {
    if (logic == nullptr) return;
    logic->restart();
    undoStack.clear();
    undoUi.clear();
    marks.clear();
    answer = 0;
    selectedNode = 0;
    wrongNode = 0;
    pendingResult = false;
    clearPan();
    refreshTokens();
    rebuildStill();
    message = "Level reset. " + objectiveFor(logic->definition());
}

void TreeMissionScreen::useHint() {
    if (logic == nullptr || pendingResult) return;
    PlayerAction unused;
    pushUndo(unused);
    PlayResult result = logic->hint();
    message = "HINT: " + describeAction(result.hintAction);
    if (logic->isFailed()) armResult();
}

string TreeMissionScreen::caseName(int choice) const {
    if (logic == nullptr) return to_string(choice);
    const string& task = logic->definition().task;
    if (task == "bst_delete") {
        if (choice == DeleteCase::LEAF) return "leaf";
        if (choice == DeleteCase::ONE_CHILD) return "one child";
        if (choice == DeleteCase::TWO_CHILD) return "two child";
    }
    if (choice < 0) return "none";
    if (choice == RotationType::LL) return "LL";
    if (choice == RotationType::RR) return "RR";
    if (choice == RotationType::LR) return "LR";
    if (choice == RotationType::RL) return "RL";
    return to_string(choice);
}

string TreeMissionScreen::describeAction(const PlayerAction& action) const {
    if (logic == nullptr) return "";
    if (action.kind == ActionKind::CLICK_NODE) {
        TreeNode* node = logic->tree().findById(action.id);
        if (node != nullptr) return "click " + to_string(node->value);
        return "click node " + to_string(action.id);
    }
    if (action.kind == ActionKind::PLACE_TOKEN) {
        return "place " + to_string(action.value) + " in slot " + to_string(action.slot + 1);
    }
    if (action.kind == ActionKind::ANSWER) return "answer " + to_string(action.value);
    if (action.kind == ActionKind::CHOOSE_CASE) return "choose " + caseName(action.choice);
    if (action.kind == ActionKind::CONFIRM) return "confirm";
    if (action.kind == ActionKind::AUTO) return "auto";
    return "next step";
}

string TreeMissionScreen::objectiveFor(const LevelDef& def) {
    const string& task = def.task;
    const LevelRules& rules = def.rules;
    if (task == "bst_insert") {
        return "Insert " + to_string(rules.target) + ". Click its search path, then confirm.";
    }
    if (task == "bst_search") {
        return "Search for " + to_string(rules.target) + ". Click every room the search enters.";
    }
    if (task == "bst_delete") {
        return "Delete " + to_string(rules.target) + ". Name its case, then confirm.";
    }
    if (task == "bst_range") {
        return "Click every key from " + to_string(rules.lo) + " to " + to_string(rules.hi) + ".";
    }
    if (task == "lca_path") {
        return "Lowest common room of " + to_string(rules.a) + " and " + to_string(rules.b) +
               ", then the path length.";
    }
    if (task == "avl_kth") {
        return "Type kth(n-k+1) with k = " + to_string(rules.k) +
               " (2nd largest here), then click the repair pivot. Twice.";
    }
    if (task == "traversal_click" && !rules.traversal.empty()) {
        return "Click the rooms in " + rules.traversal + ".";
    }
    if (task == "avl_badges") {
        return "Click every badge in preorder: root, left, right.";
    }
    if (task == "avl_rotate_single") {
        return "Click each unbalanced pivot. This level is LL / RR only.";
    }
    if (task == "avl_rotate_double") {
        return "Name LR or RL, then click that pivot. Four crooked subtrees.";
    }
    if (task == "bst_boss") {
        return "Boss chain: click three search paths. Scroll zooms, drag pans.";
    }
    if (task == "avl_auto_predict") {
        string line = "Predict the rotation for";
        for (int i = 0; i < rules.inserts.size(); i++) {
            line += i == 0 ? " " : ", ";
            line += to_string(rules.inserts[i]);
        }
        return line + ".";
    }
    return def.mission;
}

string TreeMissionScreen::instruction() const {
    if (logic == nullptr) return "No level loaded";
    const string& task = logic->definition().task;
    const LevelRules& rules = logic->definition().rules;
    PlayerAction next = logic->nextAction();
    if (next.kind == ActionKind::ANSWER) {
        if (task == "height_diameter") return "Type the height, then the diameter. Enter submits.";
        if (task == "lca_path") return "Type the common room, then the number of edges. Enter submits.";
        if (task == "avl_kth") {
            int n = logic->tree().size();
            int position = n - rules.k + 1;
            if (position < 1) position = 1;
            return "Type the " + to_string(position) + "th smallest key (n-k+1). Enter submits.";
        }
        return "Type the answer. Enter submits.";
    }
    if (next.kind == ActionKind::PLACE_TOKEN) return "Select a token, then the preorder slot it belongs in.";
    if (next.kind == ActionKind::CHOOSE_CASE) {
        if (task == "bst_delete") return "What case is " + to_string(rules.target) + "? Leaf, one child, or two.";
        if (task == "avl_rotate_double") {
            return "Name LR or RL for the deepest crooked subtree.";
        }
        return "Predict LL, RR, LR, RL, or none.";
    }
    if (next.kind == ActionKind::CONFIRM) {
        if (task == "bst_insert") return "Path complete. Confirm to insert " + to_string(rules.target) + ".";
        if (task == "bst_delete") return "Case accepted. Confirm to delete " + to_string(rules.target) + ".";
        return "Confirm the step.";
    }
    if (task == "mirror") return "Click an internal node to swap its children.";
    if (task == "traversal_click") {
        string order = rules.traversal.empty() ? "visit order" : rules.traversal;
        return "Click the next room in " + order + ". Visited rooms stay marked.";
    }
    if (task == "bst_insert") return "Click the next room on the path to " + to_string(rules.target) + ".";
    if (task == "bst_search") return "Click the next room on the path to " + to_string(rules.target) + ".";
    if (task == "bst_boss") {
        return "Click the next room on the path. Drag empty space to pan.";
    }
    if (task == "bst_range") {
        return "Click the next key from " + to_string(rules.lo) + " to " + to_string(rules.hi) + ".";
    }
    if (task == "avl_badges") return "Click the next badge in preorder.";
    if (task.find("rotate") != string::npos || task == "avl_kth") {
        if (next.kind == ActionKind::CLICK_NODE) {
            TreeNode* node = logic->tree().findById(next.id);
            if (node != nullptr) {
                return "Next pivot is " + to_string(node->value) +
                       ". Click it, or select it and press Rotate.";
            }
        }
        return "Click the unbalanced pivot, or select it and press Rotate.";
    }
    return "Click the next node.";
}

sf::Vector2f TreeMissionScreen::worldAt(sf::Vector2f screen) const {
    sf::FloatRect field = playfield();
    float localX = screen.x - field.position.x;
    float localY = screen.y - field.position.y;
    return center + sf::Vector2f((localX - field.size.x / 2.f) * scale,
                                 (localY - field.size.y / 2.f) * scale);
}

void TreeMissionScreen::clearPan() {
    panning = false;
    panMoved = false;
    panLast = {0.f, 0.f};
}

void TreeMissionScreen::applyPlayfieldView(sf::RenderWindow& window) const {
    sf::FloatRect field = playfield();
    sf::View view(center, {field.size.x * scale, field.size.y * scale});
    sf::Vector2f win = window.getDefaultView().getSize();
    if (win.x < 1.f) win.x = 1.f;
    if (win.y < 1.f) win.y = 1.f;
    view.setViewport(sf::FloatRect({field.position.x / win.x, field.position.y / win.y},
                                   {field.size.x / win.x, field.size.y / win.y}));
    window.setView(view);
}

int TreeMissionScreen::handleEvent(const sf::Event& event, sf::RenderWindow& window) {
    screenTarget = ScreenId::NONE;
    if (logic == nullptr) {
        const sf::Event::KeyPressed* key = event.getIf<sf::Event::KeyPressed>();
        if (key != nullptr && key->code == sf::Keyboard::Key::Escape) return ScreenId::CAMPAIGN_MAP;
        return ScreenId::NONE;
    }

    const sf::Event::KeyPressed* key = event.getIf<sf::Event::KeyPressed>();
    if (key != nullptr) {
        clearPan();
        if (key->code == sf::Keyboard::Key::Escape) return ScreenId::CAMPAIGN_MAP;
        if (key->code == sf::Keyboard::Key::Z) {
            undo();
            return ScreenId::NONE;
        }
        if (key->code == sf::Keyboard::Key::H) {
            useHint();
            return ScreenId::NONE;
        }
        if (key->code == sf::Keyboard::Key::Space) {
            if (animation.isPlaying()) animation.pause();
            else animation.play();
            return ScreenId::NONE;
        }
        if (key->code == sf::Keyboard::Key::Right) {
            animation.stepForward();
            return ScreenId::NONE;
        }
        if (key->code == sf::Keyboard::Key::Left) {
            animation.stepBackward();
            return ScreenId::NONE;
        }
        PlayerAction next = logic->nextAction();
        if (next.kind == ActionKind::ANSWER && !pendingResult) {
            if (key->code >= sf::Keyboard::Key::Num0 && key->code <= sf::Keyboard::Key::Num9) {
                int digit = static_cast<int>(key->code) - static_cast<int>(sf::Keyboard::Key::Num0);
                if (answer < 1000000) answer = answer * 10 + digit;
                return ScreenId::NONE;
            }
            if (key->code == sf::Keyboard::Key::Backspace) {
                answer /= 10;
                return ScreenId::NONE;
            }
            if (key->code == sf::Keyboard::Key::Enter) {
                apply(vh::answer(answer));
                answer = 0;
                return ScreenId::NONE;
            }
        }
        return ScreenId::NONE;
    }

    const sf::Event::MouseWheelScrolled* wheel = event.getIf<sf::Event::MouseWheelScrolled>();
    if (wheel != nullptr && wheel->delta != 0.f) {
        sf::Vector2f point = uiPoint(window, wheel->position);
        if (hit(playfield(), point)) {
            sf::Vector2f before = worldAt(point);
            scale = wheel->delta > 0.f ? scale * 0.9f : scale * 1.1f;
            if (scale < 0.05f) scale = 0.05f;
            if (scale > 12.f) scale = 12.f;
            sf::Vector2f after = worldAt(point);
            center += before - after;
        }
        return ScreenId::NONE;
    }

    const sf::Event::MouseMoved* moved = event.getIf<sf::Event::MouseMoved>();
    if (moved != nullptr) {
        if (!panning) return ScreenId::NONE;
        sf::Vector2f point = uiPoint(window, moved->position);
        float dx = point.x - panLast.x;
        float dy = point.y - panLast.y;
        if (!panMoved) {
            float travel = dx * dx + dy * dy;
            if (travel < 36.f) return ScreenId::NONE;
            panMoved = true;
        }
        center += {-dx * scale, -dy * scale};
        panLast = point;
        return ScreenId::NONE;
    }

    const sf::Event::MouseButtonReleased* release = event.getIf<sf::Event::MouseButtonReleased>();
    if (release != nullptr) {
        if (release->button != sf::Mouse::Button::Left || !panning) return ScreenId::NONE;
        sf::Vector2f point = uiPoint(window, release->position);
        bool wasPan = panMoved;
        clearPan();
        if (!wasPan && !pendingResult && hit(playfield(), point)) handleNode(point);
        return ScreenId::NONE;
    }

    const sf::Event::MouseButtonPressed* click = event.getIf<sf::Event::MouseButtonPressed>();
    if (click == nullptr || click->button != sf::Mouse::Button::Left) return ScreenId::NONE;
    sf::Vector2f point = uiPoint(window, click->position);
    if (handleToolbar(point)) {
        clearPan();
        return screenTarget;
    }
    if (!pendingResult && handleActions(point)) {
        clearPan();
        return ScreenId::NONE;
    }
    if (hit(playfield(), point)) {
        panning = true;
        panMoved = false;
        panLast = point;
        return ScreenId::NONE;
    }
    clearPan();
    return ScreenId::NONE;
}

bool TreeMissionScreen::handleToolbar(sf::Vector2f point) {
    if (hit(LevelChrome::undoButton(), point)) {
        undo();
        return true;
    }
    if (hit(LevelChrome::hintButton(), point)) {
        useHint();
        return true;
    }
    if (hit(LevelChrome::replayButton(), point)) {
        if (animation.eventCount() > 0) {
            animation.restart();
            animation.play();
            message = "Replaying the last step";
        } else {
            message = "Nothing to replay yet";
        }
        return true;
    }
    if (hit(LevelChrome::resetButton(), point)) {
        resetLevel();
        return true;
    }
    if (hit(LevelChrome::slowerButton(), point)) {
        animation.setSpeed(animation.getSpeed() <= 0.5 ? 1.0 : 0.5);
        message = animation.getSpeed() < 0.75 ? "Playback 0.5x" : "Playback 1x";
        return true;
    }
    if (hit(LevelChrome::fasterButton(), point)) {
        animation.setSpeed(animation.getSpeed() >= 2.0 ? 1.0 : 2.0);
        message = animation.getSpeed() > 1.5 ? "Playback 2x" : "Playback 1x";
        return true;
    }
    if (hit(LevelChrome::exitButton(), point)) {
        screenTarget = ScreenId::CAMPAIGN_MAP;
        return true;
    }
    return false;
}

bool TreeMissionScreen::handleActions(sf::Vector2f point) {
    if (logic == nullptr) return false;
    PlayerAction next = logic->nextAction();
    const string& task = logic->definition().task;

    if (next.kind == ActionKind::PLACE_TOKEN) {
        for (int i = 0; i < tokens.size() && i < 12; i++) {
            if (!hit(tokenRect(i), point)) continue;
            selectedToken = i;
            message = "Token " + to_string(tokens[i]) + " selected";
            return true;
        }
        for (int i = 0; i < tokens.size() && i < 14; i++) {
            if (!hit(slotRect(i), point)) continue;
            selectedSlot = i;
            if (selectedToken < 0 || selectedToken >= tokens.size()) {
                message = "Select a token first";
                return true;
            }
            apply(placeToken(i, tokens[selectedToken]));
            return true;
        }
    }

    if (next.kind == ActionKind::CHOOSE_CASE) {
        if (task == "bst_delete") {
            for (int i = 0; i < 3; i++) {
                if (hit(caseRect(i, 120.f), point)) {
                    apply(chooseCase(i));
                    return true;
                }
            }
        } else if (task == "avl_rotate_double") {
            for (int i = 0; i < 2; i++) {
                if (hit(caseRect(i, 120.f), point)) {
                    apply(chooseCase(i + RotationType::LR));
                    return true;
                }
            }
        } else if (task == "avl_auto_predict") {
            for (int i = 0; i < 5; i++) {
                if (hit(caseRect(i, 96.f), point)) {
                    apply(chooseCase(i == 4 ? -1 : i));
                    return true;
                }
            }
        }
    }

    if (next.kind == ActionKind::CONFIRM && hit(submitRect(), point)) {
        apply(confirmAction());
        return true;
    }
    if (next.kind == ActionKind::ANSWER && hit(submitRect(), point)) {
        apply(vh::answer(answer));
        answer = 0;
        return true;
    }
    if (next.kind == ActionKind::CLICK_NODE && task.find("rotate") != string::npos &&
        hit(submitRect(), point)) {
        if (selectedNode == 0) {
            message = "Select the pivot first";
            return true;
        }
        apply(clickNode(selectedNode));
        return true;
    }
    if (task == "mirror" && hit(submitRect(), point)) {
        apply(autoAction());
        return true;
    }
    return false;
}

void TreeMissionScreen::handleNode(sf::Vector2f point) {
    if (logic == nullptr) return;
    syncView();
    int nodeId = 0;
    if (!treeView.pickNode(worldAt(point), nodeId)) return;
    selectedNode = nodeId;
    TreeNode* node = logic->tree().findById(nodeId);
    int value = node == nullptr ? nodeId : node->value;
    PlayerAction next = logic->nextAction();
    if (next.kind == ActionKind::CLICK_NODE) {
        apply(clickNode(nodeId));
        return;
    }
    wrongNode = 0;
    message = "Selected " + to_string(value);
}

int TreeMissionScreen::update(float seconds) {
    animation.update(seconds);
    if (!pendingResult || logic == nullptr) return ScreenId::NONE;
    if (animation.isPlaying()) return ScreenId::NONE;
    if (animation.completedCount() < animation.eventCount()) return ScreenId::NONE;
    pendingResult = false;
    return ScreenId::RESULT;
}

void TreeMissionScreen::drawRings(sf::RenderWindow& window) const {
    TreeLayout frame = treeView.currentLayout();
    for (int i = 0; i < frame.size(); i++) {
        int id = frame.at(i).nodeId;
        if (!marked(id) || id == wrongNode) continue;
        float x = frame.xOf(id);
        float y = frame.yOf(id);
        sf::CircleShape ring(30.f);
        ring.setOrigin({30.f, 30.f});
        ring.setPosition({x, y});
        ring.setFillColor(sf::Color::Transparent);
        ring.setOutlineThickness(3.f);
        ring.setOutlineColor(Palette::VISITED);
        window.draw(ring);
    }
    int focus = wrongNode != 0 ? wrongNode : selectedNode;
    bool showFocus = focus != 0 && frame.contains(focus) && (wrongNode != 0 || !marked(focus));
    if (!showFocus) return;
    sf::CircleShape ring(34.f);
    ring.setOrigin({34.f, 34.f});
    ring.setPosition({frame.xOf(focus), frame.yOf(focus)});
    ring.setFillColor(sf::Color::Transparent);
    ring.setOutlineThickness(4.f);
    ring.setOutlineColor(wrongNode != 0 ? Palette::VIOLATION : Palette::TARGET);
    window.draw(ring);
}

void TreeMissionScreen::drawActions(sf::RenderWindow& window, Renderer& renderer) {
    if (logic == nullptr || pendingResult || logic->isComplete() || logic->isFailed()) return;
    PlayerAction next = logic->nextAction();
    const string& task = logic->definition().task;

    if (next.kind == ActionKind::PLACE_TOKEN) {
        renderer.drawText("TOKENS", {40.f, 500.f}, 11, Palette::MUTED, false);
        for (int i = 0; i < tokens.size() && i < 12; i++) {
            int kind = i == selectedToken ? ButtonKind::PRIMARY : ButtonKind::SECONDARY;
            theme.drawButton(window, theme.getFont(), tokenRect(i), to_string(tokens[i]), kind,
                             theme.hovered(window, tokenRect(i)));
        }
        renderer.drawText("SLOTS", {40.f, 548.f}, 11, Palette::MUTED, false);
        for (int i = 0; i < tokens.size() && i < 14; i++) {
            string label = slotFilled[i] != 0 ? to_string(slotValue[i]) : to_string(i + 1);
            int kind = slotFilled[i] != 0 ? ButtonKind::PRIMARY
                                           : (i == selectedSlot ? ButtonKind::PRIMARY : ButtonKind::GHOST);
            theme.drawButton(window, theme.getFont(), slotRect(i), label, kind,
                             theme.hovered(window, slotRect(i)));
        }
        return;
    }

    if (next.kind == ActionKind::CHOOSE_CASE && task == "bst_delete") {
        const char* labels[3] = {"LEAF", "ONE CHILD", "TWO CHILD"};
        for (int i = 0; i < 3; i++) {
            theme.drawButton(window, theme.getFont(), caseRect(i, 120.f), labels[i], ButtonKind::SECONDARY,
                             theme.hovered(window, caseRect(i, 120.f)));
        }
        return;
    }
    if (next.kind == ActionKind::CHOOSE_CASE && task == "avl_rotate_double") {
        const char* labels[2] = {"LR", "RL"};
        for (int i = 0; i < 2; i++) {
            theme.drawButton(window, theme.getFont(), caseRect(i, 120.f), labels[i], ButtonKind::SECONDARY,
                             theme.hovered(window, caseRect(i, 120.f)));
        }
        return;
    }
    if (next.kind == ActionKind::CHOOSE_CASE && task == "avl_auto_predict") {
        const char* labels[5] = {"LL", "RR", "LR", "RL", "NONE"};
        for (int i = 0; i < 5; i++) {
            theme.drawButton(window, theme.getFont(), caseRect(i, 96.f), labels[i], ButtonKind::SECONDARY,
                             theme.hovered(window, caseRect(i, 96.f)));
        }
        return;
    }

    if (next.kind == ActionKind::CONFIRM) {
        string label = "Confirm";
        if (task == "bst_insert") label = "Insert " + to_string(logic->definition().rules.target);
        if (task == "bst_delete") label = "Delete " + to_string(logic->definition().rules.target);
        theme.drawButton(window, theme.getFont(), submitRect(), label, ButtonKind::PRIMARY,
                         theme.hovered(window, submitRect()));
        return;
    }
    if (next.kind == ActionKind::ANSWER) {
        theme.drawButton(window, theme.getFont(), caseRect(0, 180.f), "Value  " + to_string(answer),
                         ButtonKind::GHOST, false);
        theme.drawButton(window, theme.getFont(), submitRect(), "Submit", ButtonKind::PRIMARY,
                         theme.hovered(window, submitRect()));
        return;
    }
    if (next.kind == ActionKind::CLICK_NODE && task.find("rotate") != string::npos) {
        string label = selectedNode == 0 ? "Rotate" : "Rotate " + to_string(selectedNode);
        TreeNode* node = logic->tree().findById(selectedNode);
        if (node != nullptr) label = "Rotate " + to_string(node->value);
        theme.drawButton(window, theme.getFont(), submitRect(), label, ButtonKind::PRIMARY,
                         theme.hovered(window, submitRect()));
        return;
    }
    if (task == "mirror") {
        theme.drawButton(window, theme.getFont(), submitRect(), "Auto", ButtonKind::GHOST,
                         theme.hovered(window, submitRect()));
    }
}

void TreeMissionScreen::draw(sf::RenderWindow& window) {
    theme.drawBackdrop(window);
    Renderer renderer(window, theme.getFont());
    chrome.drawCanvasPanel(window, theme);
    chrome.drawSidePanel(window, theme);

    if (logic == nullptr) {
        renderer.drawText("Level data unavailable", {400.f, 300.f}, 20, Palette::VIOLATION, true);
        return;
    }

    const LevelDef& def = logic->definition();
    renderer.drawText(LevelChrome::clip(objectiveFor(def), 78), {48.f, 44.f}, 15, Palette::TEXT, false);
    renderer.drawText(LevelChrome::clip(instruction(), 78), {48.f, 72.f}, 13, Palette::BRASS, false);

    syncView();
    applyPlayfieldView(window);
    treeView.draw(renderer);
    drawRings(window);
    window.setView(window.getDefaultView());

    drawActions(window, renderer);
    chrome.drawHud(window, renderer, theme, logic->score(), def.id, def.title, instruction(), message,
                   logic->isFailed());
    int highlight = animation.completedCount() - 1;
    chrome.drawEventLog(renderer, animation.log(), highlight);

    chrome.drawButton(window, theme, renderer, LevelChrome::undoButton(), "Undo");
    chrome.drawButton(window, theme, renderer, LevelChrome::hintButton(), "Hint");
    chrome.drawButton(window, theme, renderer, LevelChrome::replayButton(), "Replay");
    chrome.drawButton(window, theme, renderer, LevelChrome::resetButton(), "Reset", ButtonKind::DANGER);
    chrome.drawButton(window, theme, renderer, LevelChrome::slowerButton(), "Slow", ButtonKind::GHOST);
    chrome.drawButton(window, theme, renderer, LevelChrome::fasterButton(), "Fast", ButtonKind::GHOST);
    chrome.drawButton(window, theme, renderer, LevelChrome::exitButton(), "Campaign", ButtonKind::SECONDARY);
}

}
