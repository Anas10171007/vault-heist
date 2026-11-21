#include "gui/screens/LevelScreen.h"
#include "game/LevelPlayFactory.h"
#include "gui/core/Renderer.h"
#include "gui/core/ScreenId.h"
#include "game/tree/TreeLayout.h"

namespace vh {
LevelScreen::LevelScreen(Theme& sharedTheme, GameSession& sharedSession)
    : theme(sharedTheme), session(sharedSession), heapScreen(sharedTheme), logic(nullptr), heapMode(false),
      viewZoom(1.f), viewX(0.f), viewY(0.f) {
    const char* paths[] = {"data/levels", "../data/levels", "../../data/levels"};
    for (const char* path : paths) if (levels.loadAll(path)) break;
}
LevelScreen::~LevelScreen() { delete logic; }
sf::FloatRect LevelScreen::nextButton() const { return {{810.f, 400.f}, {240.f, 42.f}}; }
sf::FloatRect LevelScreen::hintButton() const { return {{810.f, 448.f}, {240.f, 38.f}}; }
sf::FloatRect LevelScreen::undoButton() const { return {{810.f, 492.f}, {240.f, 38.f}}; }
sf::FloatRect LevelScreen::resetButton() const { return {{810.f, 536.f}, {240.f, 38.f}}; }
sf::FloatRect LevelScreen::exitButton() const { return {{810.f, 580.f}, {240.f, 38.f}}; }

void LevelScreen::onShow() {
    delete logic;
    logic = nullptr;
    viewZoom = 1.f; viewX = 0.f; viewY = 0.f;
    const LevelDef* level = levels.load(session.selectedLevel);
    heapMode = level && level->act == 4;
    if (!level) { message = "Level data unavailable."; return; }
    if (heapMode) {
        heapScreen.setAnimationSpeed(session.animationSpeed);
        heapScreen.selectLevel(level->id);
        message.clear();
        return;
    }
    logic = createLevelPlay(*level);
    if (!logic || !logic->load(*level)) { message = "Could not create level logic."; return; }
    history.clear();
    message = "Click Next Step, or press N, to execute the expected player action.";
}

void LevelScreen::applyNext() {
    if (!logic || logic->isComplete() || logic->isFailed()) return;
    PlayerAction action = logic->nextAction();
    history.push(action);
    PlayResult result = logic->apply(action);
    message = result.message.empty() ? (result.correct ? "Correct action." : "Action applied.") : result.message;
}

void LevelScreen::undo() {
    if (!logic || history.empty()) { message = "Nothing to undo."; return; }
    history.pop();
    DynArray<PlayerAction> replay = history;
    logic->reset();
    for (int index = 0; index < replay.size(); index++) logic->apply(replay[index]);
    message = "Last action undone.";
}

void LevelScreen::storeResult(const ScoreSystem& score) {
    session.score = score.score();
    session.stars = score.stars();
    session.steps = score.steps();
    session.penalties = score.penalties();
    session.resultMessage = "Mission " + session.selectedLevel + " complete";
}

int LevelScreen::handleEvent(const sf::Event& event, sf::RenderWindow& window) {
    if (heapMode) {
        int target = heapScreen.handleEvent(event, window);
        if (target == ScreenId::RESULT) storeResult(heapScreen.score());
        return target;
    }
    const auto* key = event.getIf<sf::Event::KeyPressed>();
    if (key) {
        if (key->code == sf::Keyboard::Key::N) applyNext();
        else if (key->code == sf::Keyboard::Key::Z) undo();
        else if (key->code == sf::Keyboard::Key::Q && viewZoom > 0.55f) viewZoom -= 0.1f;
        else if (key->code == sf::Keyboard::Key::E && viewZoom < 1.8f) viewZoom += 0.1f;
        else if (key->code == sf::Keyboard::Key::A) viewX -= 20.f;
        else if (key->code == sf::Keyboard::Key::D) viewX += 20.f;
        else if (key->code == sf::Keyboard::Key::W) viewY -= 20.f;
        else if (key->code == sf::Keyboard::Key::S) viewY += 20.f;
        else if (key->code == sf::Keyboard::Key::H && logic) {
            PlayResult result = logic->hint();
            message = result.message.empty() ? "Hint highlighted the next action." : result.message;
        } else if (key->code == sf::Keyboard::Key::R && logic) { logic->reset(); history.clear(); }
        else if (key->code == sf::Keyboard::Key::Enter && logic && logic->isComplete()) {
            storeResult(logic->score()); return ScreenId::RESULT;
        }
    }
    const auto* click = event.getIf<sf::Event::MouseButtonPressed>();
    if (!click || click->button != sf::Mouse::Button::Left) return ScreenId::NONE;
    sf::Vector2f point = window.mapPixelToCoords(click->position);
    if (nextButton().contains(point)) applyNext();
    else if (hintButton().contains(point) && logic) {
        PlayResult result = logic->hint();
        message = result.message.empty() ? "Hint highlighted the next action." : result.message;
    } else if (undoButton().contains(point)) undo();
    else if (resetButton().contains(point) && logic) { logic->reset(); history.clear(); }
    else if (exitButton().contains(point)) return ScreenId::CAMPAIGN_MAP;
    if (logic && logic->isComplete()) { storeResult(logic->score()); return ScreenId::RESULT; }
    return ScreenId::NONE;
}

void LevelScreen::update(float seconds) { if (heapMode) heapScreen.update(seconds); }

void LevelScreen::draw(sf::RenderWindow& window) {
    if (heapMode) { heapScreen.draw(window); return; }
    Renderer renderer(window, theme.getFont());
    window.draw(theme.makePanel({40.f, 45.f}, {720.f, 610.f}));
    window.draw(theme.makePanel({785.f, 45.f}, {290.f, 610.f}));
    if (!logic) { renderer.drawText(message, {550.f, 330.f}, 20, Palette::VIOLATION, true); return; }
    const LevelDef& level = logic->definition();
    renderer.drawText(level.id + "  " + level.title, {70.f, 75.f}, 26, Palette::TARGET, false);
    renderer.drawText(level.mission, {70.f, 125.f}, 15, Palette::TEXT, false);
    renderer.drawText("MODEL STATE", {400.f, 175.f}, 18, Palette::MUTED, true);
    TreeLayout layout = TreeLayout::compute(logic->tree(), 70.f, 82.f);
    float centre = (layout.bounds().minX + layout.bounds().maxX) / 2.f;
    DynArray<const TreeNode*> stack;
    if (logic->tree().root()) stack.push(logic->tree().root());
    while (!stack.empty()) {
        const TreeNode* node = stack.pop();
        sf::Vector2f from(400.f + viewX + (layout.xOf(node->id) - centre) * viewZoom,
                          235.f + viewY + layout.yOf(node->id) * viewZoom);
        if (node->left) {
            sf::Vector2f to(400.f + viewX + (layout.xOf(node->left->id) - centre) * viewZoom,
                            235.f + viewY + layout.yOf(node->left->id) * viewZoom);
            renderer.drawEdge(from, to, false); stack.push(node->left);
        }
        if (node->right) {
            sf::Vector2f to(400.f + viewX + (layout.xOf(node->right->id) - centre) * viewZoom,
                            235.f + viewY + layout.yOf(node->right->id) * viewZoom);
            renderer.drawEdge(from, to, false); stack.push(node->right);
        }
    }
    for (int i = 0; i < layout.size(); i++) {
        const LayoutPoint& point = layout.at(i);
        const TreeNode* node = logic->tree().findById(point.nodeId);
        renderer.drawNode({400.f + viewX + (point.x - centre) * viewZoom,
                           235.f + viewY + point.y * viewZoom},
                          node ? to_string(node->value) : "?", 0);
    }
    renderer.drawText(message, {400.f, 350.f}, 14,
                      logic->isFailed() ? Palette::VIOLATION : Palette::WARNING, true);
    alarm.draw(window, renderer, logic->score(), {820.f, 170.f}, 220.f);
    renderer.drawText("Steps: " + to_string(logic->score().steps()), {820.f, 230.f}, 16, Palette::TEXT, false);
    renderer.drawText("Score: " + to_string(logic->score().score()), {820.f, 260.f}, 16, Palette::TEXT, false);
    const sf::FloatRect buttons[] = {nextButton(), hintButton(), undoButton(), resetButton(), exitButton()};
    const char* labels[] = {"Next Step [N]", "Hint [H]", "Undo [Z]", "Reset [R]", "Campaign Map"};
    for (int index = 0; index < 5; index++) {
        window.draw(theme.makeButton(buttons[index].position, buttons[index].size));
        renderer.drawText(labels[index], buttons[index].position + buttons[index].size / 2.f,
                          17, Palette::TEXT, true);
    }
    if (session.showHelp) renderer.drawText("N next | H hint | Z undo | Q/E zoom | WASD pan",
                                            {400.f, 625.f}, 13, Palette::MUTED, true);
}
}
