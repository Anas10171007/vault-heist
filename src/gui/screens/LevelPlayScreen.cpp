#include "gui/screens/LevelPlayScreen.h"

#include <string>

#include "game/LevelDataBuilder.h"
#include "game/LevelPlayFactory.h"
#include "gui/core/Renderer.h"
#include "gui/core/Style.h"
#include "gui/widgets/AlarmMeter.h"
#include "model/Node.h"

namespace vh {

namespace {

bool contains(const sf::FloatRect& rect, sf::Vector2f point) {
    return rect.contains(point);
}

sf::FloatRect rect(float x, float y, float w, float h) {
    return sf::FloatRect({x, y}, {w, h});
}

}

LevelPlayScreen::LevelPlayScreen(Theme& sharedTheme)
    : theme(sharedTheme), camera({1100.f, 720.f}) {
    manager.loadAll("data/levels");
    treeView.setOrigin(60.f, 120.f);
    animation.setListener(&treeView);
}

LevelPlayScreen::~LevelPlayScreen() {
    delete logic;
}

bool LevelPlayScreen::loadLevel(const std::string& id) {
    const LevelDef* definition = manager.load(id);
    if (definition == nullptr) return false;
    LevelPlay* replacement = createLevelPlay(*definition);
    if (replacement == nullptr || !replacement->load(*definition)) {
        delete replacement;
        return false;
    }
    delete logic;
    logic = replacement;
    logic->restart();
    levelId = id;
    nextLevelId.clear();
    undoStack.clear();
    logLines.clear();
    tokens.clear();
    selectedNode = 0;
    selectedToken = -1;
    selectedSlot = -1;
    answer = 0;
    answerReady = false;
    overlay = false;
    hintText.clear();
    message = definition->mission;
    refreshTokens();
    rebuildView();
    return true;
}

void LevelPlayScreen::setNextLevel(const std::string& id) {
    nextLevelId = id;
}

void LevelPlayScreen::refreshTokens() {
    tokens.clear();
    if (logic == nullptr) return;
    const LevelData& data = logic->definition().data;
    if (data.hasValues) {
        for (int i = 0; i < data.values.size(); i++) tokens.push(data.values[i]);
        return;
    }
    if (data.hasTree) {
        BinaryTree source;
        source.deserialize(data.tree);
        BinaryTree::TraversalResult result = source.preorder();
        for (int i = 0; i < result.ids.size(); i++) {
            TreeNode* node = source.findById(result.ids[i]);
            if (node != nullptr) tokens.push(node->value);
        }
        return;
    }
    DynArray<int> values;
    LevelDataBuilder::buildValues(logic->definition(), values);
    for (int i = 0; i < values.size(); i++) tokens.push(values[i]);
}

void LevelPlayScreen::rebuildView() {
    if (logic == nullptr) return;
    treeView.setModel(logic->tree());
    TreeLayout layout = TreeLayout::compute(logic->tree());
    treeView.setMotion(TreeMotion::simple(layout, EventLog(), layout));
    animation.restart();
    fitCamera();
}

void LevelPlayScreen::fitCamera() {
    if (logic == nullptr) return;
    TreeLayout layout = treeView.currentLayout();
    if (layout.empty()) return;
    sf::FloatRect bounds({layout.bounds().minX + 60.f - 70.f, layout.bounds().minY + 120.f - 70.f},
                         {layout.bounds().width() + 140.f, layout.bounds().height() + 140.f});
    camera.fitToView(bounds, 40.f);
}

std::string LevelPlayScreen::describe(const Event& event) const {
    std::string line = eventName(event.type);
    if (event.type == EventType::ROTATE) {
        line += " " + std::string(event.extra == RotationType::LL ? "LL"
                                 : event.extra == RotationType::RR ? "RR"
                                 : event.extra == RotationType::LR ? "LR" : "RL");
    }
    if (event.idA != 0) line += " node " + std::to_string(event.idA);
    if (event.idB != 0) line += " -> " + std::to_string(event.idB);
    if (event.type == EventType::COMPARE || event.type == EventType::INSERT ||
        event.type == EventType::BALANCE_ALERT) {
        line += " key " + std::to_string(event.key);
    }
    if (event.type == EventType::SWAP_VALUE) {
        line += " " + std::to_string(event.oldValue) + "/" + std::to_string(event.newValue);
    }
    return line;
}

void LevelPlayScreen::appendEvents(const EventLog& events) {
    for (int i = 0; i < events.size(); i++) {
        if (events.at(i).type == EventType::DONE) continue;
        logLines.push(describe(events.at(i)));
    }
    while (logLines.size() > 9) {
        DynArray<std::string> trimmed;
        for (int i = 1; i < logLines.size(); i++) trimmed.push(logLines[i]);
        logLines = trimmed;
    }
}

std::string LevelPlayScreen::actionLabel(const PlayerAction& action) const {
    if (action.kind == ActionKind::CLICK_NODE) return "SELECT NODE " + std::to_string(action.id);
    if (action.kind == ActionKind::PLACE_TOKEN) {
        return "PLACE " + std::to_string(action.value) + " IN SLOT " + std::to_string(action.slot + 1);
    }
    if (action.kind == ActionKind::ANSWER) return "ANSWER " + std::to_string(action.value);
    if (action.kind == ActionKind::CHOOSE_CASE) return "CASE " + std::to_string(action.choice);
    if (action.kind == ActionKind::CONFIRM) return "CONFIRM";
    return "AUTO";
}

void LevelPlayScreen::applyAction(const PlayerAction& action) {
    if (logic == nullptr || overlay) return;
    LevelSnapshot before = logic->captureSnapshot();
    TreeLayout beforeLayout = TreeLayout::compute(logic->tree());
    PlayResult result = logic->apply(action);
    TreeLayout afterLayout = TreeLayout::compute(logic->tree());
    undoStack.push(before);
    treeView.setMotion(TreeMotion::simple(beforeLayout, result.events, afterLayout));
    animation.load(result.events);
    animation.play();
    appendEvents(result.events);
    message = result.message.empty() ? actionLabel(action) : result.message;
    hintText.clear();
    if (result.won) {
        progress.record(levelId, logic->score().stars());
        overlay = true;
        message = "LEVEL CLEARED";
    } else if (result.lost) {
        overlay = true;
        message = "ALARM CAPACITY REACHED";
    }
}

void LevelPlayScreen::undoAction() {
    if (logic == nullptr || undoStack.empty()) return;
    logic->restoreSnapshot(undoStack.pop());
    overlay = false;
    message = "undo: restored previous state";
    hintText.clear();
    if (!logLines.empty()) {
        DynArray<std::string> trimmed;
        for (int i = 0; i + 1 < logLines.size(); i++) trimmed.push(logLines[i]);
        logLines = trimmed;
    }
    rebuildView();
}

void LevelPlayScreen::resetLevel() {
    if (logic == nullptr) return;
    logic->restart();
    undoStack.clear();
    logLines.clear();
    tokens.clear();
    selectedNode = 0;
    selectedToken = -1;
    selectedSlot = -1;
    answer = 0;
    answerReady = false;
    overlay = false;
    hintText.clear();
    message = "level reset";
    refreshTokens();
    rebuildView();
}

void LevelPlayScreen::useHint() {
    if (logic == nullptr || overlay) return;
    LevelSnapshot before = logic->captureSnapshot();
    PlayResult result = logic->hint();
    undoStack.push(before);
    PlayerAction next = result.hintAction;
    hintText = "HINT: " + actionLabel(next);
    message = result.message;
}

void LevelPlayScreen::update(float deltaSeconds) {
    animation.update(deltaSeconds);
}

void LevelPlayScreen::onShow() {
    if (logic == nullptr && !manager.loadAll("data/levels")) return;
    if (logic == nullptr) loadLevel("1.1");
    else rebuildView();
}

int LevelPlayScreen::handleEvent(const sf::Event& event, sf::RenderWindow& window) {
    camera.handleEvent(event);
    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
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
        if (key->code == sf::Keyboard::Key::Z) {
            undoAction();
            return ScreenId::NONE;
        }
        if (key->code == sf::Keyboard::Key::H) {
            useHint();
            return ScreenId::NONE;
        }
        if (key->code == sf::Keyboard::Key::Escape) return ScreenId::CAMPAIGN_MAP;
        if (key->code >= sf::Keyboard::Key::Num0 && key->code <= sf::Keyboard::Key::Num9) {
            int digit = static_cast<int>(key->code) - static_cast<int>(sf::Keyboard::Key::Num0);
            answer = answer * 10 + digit;
            answerReady = true;
            return ScreenId::NONE;
        }
        if (key->code == sf::Keyboard::Key::Backspace) {
            answer /= 10;
            return ScreenId::NONE;
        }
        if (key->code == sf::Keyboard::Key::Enter && answerReady) {
            applyAction(vh::answer(answer));
            answer = 0;
            answerReady = false;
            return ScreenId::NONE;
        }
    }

    const auto* click = event.getIf<sf::Event::MouseButtonPressed>();
    if (click == nullptr || click->button != sf::Mouse::Button::Left) return ScreenId::NONE;
    sf::Vector2f point = window.mapPixelToCoords(click->position);
    int handled = ScreenId::NONE;
    if (handleButtonClick(point, handled)) return handled;
    if (logic == nullptr || overlay || point.x > 750.f || point.y < 100.f || point.y > 555.f) {
        return ScreenId::NONE;
    }
    int nodeId = 0;
    if (treeView.pickNode(camera.toWorld(click->position), nodeId)) {
        selectedNode = nodeId;
        PlayerAction next = logic->nextAction();
        if (next.kind == ActionKind::CLICK_NODE) applyAction(clickNode(nodeId));
        else message = "node " + std::to_string(nodeId) + " selected";
    }
    return ScreenId::NONE;
}

bool LevelPlayScreen::handleButtonClick(sf::Vector2f point, int& handled) {
    if (contains(rect(20.f, 660.f, 110.f, 40.f), point)) {
        undoAction();
        return true;
    }
    if (contains(rect(140.f, 660.f, 110.f, 40.f), point)) {
        resetLevel();
        return true;
    }
    if (contains(rect(260.f, 660.f, 110.f, 40.f), point)) {
        useHint();
        return true;
    }
    if (contains(rect(380.f, 660.f, 130.f, 40.f), point)) {
        speed = speed == 0.5f ? 1.f : (speed == 1.f ? 2.f : 0.5f);
        animation.setSpeed(speed);
        message = "speed " + std::to_string(static_cast<int>(speed)) + "x";
        return true;
    }
    if (contains(rect(520.f, 660.f, 110.f, 40.f), point)) {
        handled = ScreenId::CAMPAIGN_MAP;
        return true;
    }
    if (logic == nullptr) return false;

    if (overlay) {
        if (contains(rect(390.f, 430.f, 140.f, 46.f), point)) {
            resetLevel();
            return true;
        }
        if (!nextLevelId.empty() && contains(rect(545.f, 430.f, 140.f, 46.f), point)) {
            loadLevel(nextLevelId);
            return true;
        }
        if (contains(rect(700.f, 430.f, 140.f, 46.f), point)) {
            handled = ScreenId::CAMPAIGN_MAP;
            return true;
        }
        return false;
    }

    PlayerAction next = logic->nextAction();
    if (next.kind == ActionKind::CONFIRM && contains(rect(590.f, 600.f, 130.f, 44.f), point)) {
        applyAction(vh::confirmAction());
        return true;
    }
    if (next.kind == ActionKind::AUTO && contains(rect(590.f, 600.f, 130.f, 44.f), point)) {
        applyAction(autoAction());
        return true;
    }
    if (next.kind == ActionKind::ANSWER && contains(rect(590.f, 600.f, 130.f, 44.f), point)) {
        applyAction(vh::answer(answer));
        answer = 0;
        answerReady = false;
        return true;
    }
    if (next.kind == ActionKind::CHOOSE_CASE) {
        const std::string& task = logic->definition().task;
        if (task == "bst_delete") {
            for (int i = 0; i < 3; i++) {
                if (contains(rect(20.f + i * 115.f, 600.f, 108.f, 44.f), point)) {
                    applyAction(chooseCase(i));
                    return true;
                }
            }
        } else if (task == "avl_rotate_double") {
            for (int i = 0; i < 2; i++) {
                if (contains(rect(20.f + i * 115.f, 600.f, 108.f, 44.f), point)) {
                    applyAction(chooseCase(i + RotationType::LR));
                    return true;
                }
            }
        } else if (task == "avl_auto_predict") {
            for (int i = 0; i < 5; i++) {
                if (contains(rect(20.f + i * 92.f, 600.f, 86.f, 44.f), point)) {
                    applyAction(chooseCase(i == 4 ? -1 : i));
                    return true;
                }
            }
        }
    }
    if (next.kind == ActionKind::PLACE_TOKEN) {
        for (int i = 0; i < tokens.size() && i < 12; i++) {
            if (contains(rect(20.f + (i % 12) * 48.f, 565.f, 44.f, 32.f), point)) {
                selectedToken = i;
                return true;
            }
        }
        for (int i = 0; i < tokens.size() && i < 14; i++) {
            if (contains(rect(20.f + (i % 14) * 48.f, 605.f, 44.f, 32.f), point)) {
                selectedSlot = i;
                if (selectedToken >= 0) {
                    applyAction(placeToken(i, tokens[selectedToken]));
                    selectedToken = -1;
                    selectedSlot = -1;
                }
                return true;
            }
        }
    }
    if (next.kind == ActionKind::CLICK_NODE &&
        logic->definition().task.find("rotate") != std::string::npos) {
        if (selectedNode != 0 && contains(rect(590.f, 600.f, 130.f, 44.f), point)) {
            applyAction(clickNode(selectedNode));
            return true;
        }
    }
    return false;
}

void LevelPlayScreen::drawButtons(Renderer& renderer, sf::RenderTarget& target,
                                  const RectButton* buttons, int count) const {
    for (int i = 0; i < count; i++) {
        sf::RectangleShape panel(buttons[i].bounds.size);
        panel.setPosition(buttons[i].bounds.position);
        panel.setFillColor(Palette::PANEL);
        panel.setOutlineColor(Palette::NODE_STROKE);
        panel.setOutlineThickness(2.f);
        target.draw(panel);
        renderer.drawText(buttons[i].label,
                          {buttons[i].bounds.position.x + buttons[i].bounds.size.x / 2.f,
                           buttons[i].bounds.position.y + buttons[i].bounds.size.y / 2.f},
                          15, Palette::TEXT, true);
    }
}

void LevelPlayScreen::drawHud(Renderer& renderer, sf::RenderTarget& target) {
    if (logic == nullptr) {
        renderer.drawText("No level loaded", {40.f, 40.f}, 22, Palette::TEXT, false);
        return;
    }
    const LevelDef& def = logic->definition();
    renderer.drawText(def.title + "  [" + levelId + "]", {20.f, 12.f}, 22, Palette::TARGET, false);
    renderer.drawText(def.mission, {20.f, 44.f}, 15, Palette::MUTED, false);
    renderer.drawText(message, {20.f, 78.f}, 16, Palette::TEXT, false);

    AlarmMeter meter;
    meter.draw(target, renderer, logic->score(), {20.f, 110.f}, 260.f);

    renderer.drawText("STEPS " + std::to_string(logic->score().steps()) + " / OPT " +
                          std::to_string(logic->optimalSteps()) + "  SCORE " +
                          std::to_string(logic->score().score()),
                      {300.f, 112.f}, 16, Palette::TEXT, false);
    renderer.drawText("STARS " + std::to_string(logic->score().stars()) + "  PENALTIES " +
                          std::to_string(logic->score().penalties()),
                      {300.f, 138.f}, 15, Palette::MUTED, false);
    if (!hintText.empty()) renderer.drawText(hintText, {20.f, 142.f}, 15, Palette::WARNING, false);

    renderer.drawText("EVENT LOG", {790.f, 112.f}, 16, Palette::TARGET, false);
    for (int i = 0; i < logLines.size(); i++) {
        renderer.drawText(logLines[i], {790.f, 142.f + static_cast<float>(i) * 26.f}, 14,
                          Palette::MUTED, false);
    }

    RectButton toolbar[5] = {
        {rect(20.f, 660.f, 110.f, 40.f), "UNDO Z"},
        {rect(140.f, 660.f, 110.f, 40.f), "RESET"},
        {rect(260.f, 660.f, 110.f, 40.f), "HINT H"},
        {rect(380.f, 660.f, 130.f, 40.f), "SPEED"},
        {rect(520.f, 660.f, 110.f, 40.f), "MAP ESC"}};
    drawButtons(renderer, target,toolbar, 5);

    PlayerAction next = logic->nextAction();
    if (!overlay) {
        if (next.kind == ActionKind::CONFIRM || next.kind == ActionKind::AUTO ||
            next.kind == ActionKind::ANSWER) {
            RectButton confirm = {rect(590.f, 600.f, 130.f, 44.f),
                                  next.kind == ActionKind::ANSWER
                                      ? "ANSWER " + std::to_string(answer)
                                      : (next.kind == ActionKind::CONFIRM ? "CONFIRM" : "AUTO")};
            drawButtons(renderer, target,&confirm, 1);
        } else if (next.kind == ActionKind::CHOOSE_CASE) {
            const std::string& task = logic->definition().task;
            if (task == "bst_delete") {
                RectButton cases[3] = {{rect(20.f, 600.f, 108.f, 44.f), "LEAF"},
                                       {rect(135.f, 600.f, 108.f, 44.f), "ONE CHILD"},
                                       {rect(250.f, 600.f, 108.f, 44.f), "TWO CHILD"}};
                drawButtons(renderer, target,cases, 3);
            } else if (task == "avl_rotate_double") {
                RectButton cases[2] = {{rect(20.f, 600.f, 108.f, 44.f), "LR"},
                                       {rect(135.f, 600.f, 108.f, 44.f), "RL"}};
                drawButtons(renderer, target,cases, 2);
            } else if (task == "avl_auto_predict") {
                const char* labels[5] = {"LL", "RR", "LR", "RL", "NONE"};
                RectButton cases[5];
                for (int i = 0; i < 5; i++) {
                    cases[i].bounds = rect(20.f + i * 92.f, 600.f, 86.f, 44.f);
                    cases[i].label = labels[i];
                }
                drawButtons(renderer, target,cases, 5);
            }
        } else if (next.kind == ActionKind::PLACE_TOKEN) {
            renderer.drawText("TOKENS", {20.f, 548.f}, 13, Palette::MUTED, false);
            for (int i = 0; i < tokens.size() && i < 12; i++) {
                RectButton token = {rect(20.f + (i % 12) * 48.f, 565.f, 44.f, 32.f),
                                    std::to_string(tokens[i])};
                drawButtons(renderer, target,&token, 1);
            }
            renderer.drawText("SLOTS", {20.f, 595.f}, 13, Palette::MUTED, false);
            for (int i = 0; i < tokens.size() && i < 14; i++) {
                RectButton slot = {rect(20.f + (i % 14) * 48.f, 605.f, 44.f, 32.f),
                                   std::to_string(i + 1)};
                drawButtons(renderer, target,&slot, 1);
            }
        } else if (next.kind == ActionKind::CLICK_NODE &&
                   logic->definition().task.find("rotate") != std::string::npos) {
            RectButton rotate = {rect(590.f, 600.f, 130.f, 44.f), "ROTATE " +
                                 std::to_string(selectedNode)};
            drawButtons(renderer, target,&rotate, 1);
        } else {
            renderer.drawText("SELECT A NODE TO CONTINUE", {20.f, 600.f}, 15, Palette::MUTED, false);
        }
        renderer.drawText("SPACE play/pause  LEFT/RIGHT step  Z undo  H hint  ESC map",
                          {740.f, 690.f}, 13, Palette::MUTED, false);
    }
}

void LevelPlayScreen::drawOverlay(Renderer& renderer, sf::RenderTarget& target) {
    renderer.drawText("LEVEL CLEARED", {550.f, 300.f}, 32, Palette::TARGET, true);
    renderer.drawText(message, {550.f, 340.f}, 18, Palette::TEXT, true);
    renderer.drawText("STARS " + std::to_string(logic->score().stars()) + "   STEPS " +
                          std::to_string(logic->score().steps()) + " / OPT " +
                          std::to_string(logic->optimalSteps()) + "   PENALTIES " +
                          std::to_string(logic->score().penalties()),
                      {550.f, 380.f}, 17, Palette::TEXT, true);
    RectButton retry = {rect(390.f, 430.f, 140.f, 46.f), "RETRY"};
    RectButton next = {rect(545.f, 430.f, 140.f, 46.f), "NEXT"};
    RectButton back = {rect(700.f, 430.f, 140.f, 46.f), "MAP"};
    drawButtons(renderer, target,&retry, 1);
    if (!nextLevelId.empty()) drawButtons(renderer, target,&next, 1);
    drawButtons(renderer, target,&back, 1);
}

void LevelPlayScreen::drawTo(sf::RenderTarget& target) {
    if (logic == nullptr) return;
    Renderer renderer(target, theme.getFont());
    camera.apply(target);
    treeView.draw(renderer);
    target.setView(target.getDefaultView());
    drawHud(renderer, target);
    if (overlay) drawOverlay(renderer, target);
}

void LevelPlayScreen::draw(sf::RenderWindow& window) {
    drawTo(window);
}

bool LevelPlayScreen::playAllLevels(SessionProgress& shared, DynArray<std::string>& failures) {
    LevelManager catalog;
    if (!catalog.loadAll("data/levels")) {
        failures.push("could not load level files");
        return false;
    }
    bool allOk = true;
    for (int i = 0; i < 15; i++) {
        const LevelDef& def = catalog.at(i);
        LevelPlay* play = createLevelPlay(def);
        if (play == nullptr || !play->load(def)) {
            failures.push(def.id + " did not load");
            allOk = false;
            delete play;
            continue;
        }
        play->restart();
        int guard = 0;
        while (!play->isComplete() && !play->isFailed() && guard < 200) {
            PlayResult result = play->apply(play->nextAction());
            if (!result.correct) break;
            guard++;
        }
        if (!play->isComplete() || play->isFailed() || play->score().stars() != 3) {
            failures.push(def.id + " did not win with three stars");
            allOk = false;
        } else {
            shared.record(def.id, 3);
        }
        delete play;
    }
    return allOk;
}

}
