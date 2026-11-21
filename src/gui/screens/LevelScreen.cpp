#include "gui/screens/LevelScreen.h"
#include "game/Leaderboard.h"
#include "game/LevelPlayFactory.h"
#include "game/SaveManager.h"
#include "gui/core/Renderer.h"
#include "gui/core/ScreenId.h"
#include "game/tree/TreeLayout.h"

namespace vh {
LevelScreen::LevelScreen(Theme& sharedTheme, GameSession& sharedSession)
    : theme(sharedTheme), session(sharedSession), heapScreen(sharedTheme), logic(nullptr),
      heapMode(false), viewZoom(1.f), viewX(0.f), viewY(0.f), dragging(false),
      lastDragPixel(0, 0) {
    const char* paths[] = {"data/levels", "../data/levels", "../../data/levels"};
    for (const char* path : paths) if (levels.loadAll(path)) break;
}
LevelScreen::~LevelScreen() { delete logic; }

void LevelScreen::onShow() {
    delete logic;
    logic = nullptr;
    viewZoom = 1.f; viewX = 0.f; viewY = 0.f;
    dragging = false;
    lastEvents.clear();
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
    message = "Click Next Step, or press N, to execute the expected player action.";
}

void LevelScreen::applyNext() {
    if (!logic || logic->isComplete() || logic->isFailed()) return;
    PlayerAction action = logic->nextAction();
    PlayResult result = logic->apply(action);
    lastEvents.clear();
    lastEvents.extend(result.events);
    message = result.message.empty() ? (result.correct ? "Correct action." : "Action applied.")
                                     : result.message;
}

void LevelScreen::undo() {
    if (!logic || !logic->undo()) { message = "Nothing to undo."; return; }
    lastEvents.clear();
    message = "Last action undone.";
}

void LevelScreen::showHint() {
    if (!logic) return;
    PlayResult result = logic->hint();
    message = result.message.empty() ? "Hint highlighted the next action." : result.message;
}

void LevelScreen::storeResult(const ScoreSystem& score) {
    session.score = score.score();
    session.stars = score.stars();
    session.steps = score.steps();
    session.optimalSteps = score.optimalSteps();
    session.penalties = score.penalties();
    session.resultMessage = "Mission " + session.selectedLevel + " complete";
    session.sortedArrayText.clear();
    session.finalVaultUnlocked = false;
    session.nextLevelId.clear();
    session.recordStars(session.selectedLevel, score.stars());
    SaveManager::saveProgress(session);

    Leaderboard board;
    board.load();
    board.addScore(session.selectedLevel, score.score());
    board.save();

    for (int index = 0; index < levels.size(); index++) {
        if (levels.at(index).id == session.selectedLevel && index + 1 < levels.size()) {
            session.nextLevelId = levels.at(index + 1).id;
            break;
        }
    }
}

int LevelScreen::handleEvent(const sf::Event& event, sf::RenderWindow& window) {
    if (heapMode) {
        int target = heapScreen.handleEvent(event, window);
        if (target == ScreenId::RESULT) {
            storeResult(heapScreen.score());
            if (heapScreen.isFinalVaultLevel()) {
                session.finalVaultUnlocked = true;
                session.sortedArrayText = heapScreen.sortedArrayText();
                session.resultMessage = "Final Vault Unlocked";
            }
        }
        return target;
    }

    if (const auto* wheel = event.getIf<sf::Event::MouseWheelScrolled>()) {
        if (wheel->delta > 0.f && viewZoom < 1.8f) viewZoom += 0.1f;
        if (wheel->delta < 0.f && viewZoom > 0.55f) viewZoom -= 0.1f;
        return ScreenId::NONE;
    }
    if (const auto* release = event.getIf<sf::Event::MouseButtonReleased>()) {
        if (release->button == sf::Mouse::Button::Left) dragging = false;
    }
    if (const auto* moved = event.getIf<sf::Event::MouseMoved>()) {
        if (dragging) {
            viewX += static_cast<float>(moved->position.x - lastDragPixel.x);
            viewY += static_cast<float>(moved->position.y - lastDragPixel.y);
            lastDragPixel = moved->position;
        }
    }

    const auto* key = event.getIf<sf::Event::KeyPressed>();
    if (key) {
        if (key->code == sf::Keyboard::Key::Escape) return ScreenId::CAMPAIGN_MAP;
        if (key->code == sf::Keyboard::Key::N) applyNext();
        else if (key->code == sf::Keyboard::Key::Z) undo();
        else if (key->code == sf::Keyboard::Key::Q && viewZoom > 0.55f) viewZoom -= 0.1f;
        else if (key->code == sf::Keyboard::Key::E && viewZoom < 1.8f) viewZoom += 0.1f;
        else if (key->code == sf::Keyboard::Key::A) viewX -= 20.f;
        else if (key->code == sf::Keyboard::Key::D) viewX += 20.f;
        else if (key->code == sf::Keyboard::Key::W) viewY -= 20.f;
        else if (key->code == sf::Keyboard::Key::S) viewY += 20.f;
        else if (key->code == sf::Keyboard::Key::H) showHint();
        else if (key->code == sf::Keyboard::Key::R && logic) {
            logic->reset();
            lastEvents.clear();
            message = "Level reset.";
        } else if (key->code == sf::Keyboard::Key::Enter && logic && logic->isComplete()) {
            storeResult(logic->score()); return ScreenId::RESULT;
        }
    }

    const auto* click = event.getIf<sf::Event::MouseButtonPressed>();
    if (!click || click->button != sf::Mouse::Button::Left) {
        if (logic && logic->isComplete()) { storeResult(logic->score()); return ScreenId::RESULT; }
        return ScreenId::NONE;
    }
    sf::Vector2f point = window.mapPixelToCoords(click->position);
    if (LevelChrome::opButtonWide().contains(point)) applyNext();
    else if (LevelChrome::hintButton().contains(point)) showHint();
    else if (LevelChrome::undoButton().contains(point)) undo();
    else if (LevelChrome::resetButton().contains(point) && logic) {
        logic->reset();
        lastEvents.clear();
        message = "Level reset.";
    } else if (LevelChrome::exitButton().contains(point)) return ScreenId::CAMPAIGN_MAP;
    else if (LevelChrome::canvasBounds().contains(point)) {
        dragging = true;
        lastDragPixel = click->position;
    }
    if (logic && logic->isComplete()) { storeResult(logic->score()); return ScreenId::RESULT; }
    return ScreenId::NONE;
}

int LevelScreen::update(float seconds) {
    if (!heapMode) return ScreenId::NONE;
    int target = heapScreen.update(seconds);
    if (target == ScreenId::RESULT) {
        storeResult(heapScreen.score());
        if (heapScreen.isFinalVaultLevel()) {
            session.finalVaultUnlocked = true;
            session.sortedArrayText = heapScreen.sortedArrayText();
            session.resultMessage = "Final Vault Unlocked";
        }
    }
    return target;
}

void LevelScreen::draw(sf::RenderWindow& window) {
    if (heapMode) { heapScreen.draw(window); return; }
    theme.drawBackdrop(window);
    Renderer renderer(window, theme.getFont());
    chrome.drawCanvasPanel(window, theme);
    chrome.drawSidePanel(window, theme);
    if (!logic) {
        renderer.drawText(message, {400.f, 330.f}, 20, Palette::VIOLATION, true);
        return;
    }
    const LevelDef& level = logic->definition();
    TreeLayout layout = TreeLayout::compute(logic->tree(), 70.f, 82.f);
    float centre = (layout.bounds().minX + layout.bounds().maxX) / 2.f;
    DynArray<const TreeNode*> stack;
    if (logic->tree().root()) stack.push(logic->tree().root());
    while (!stack.empty()) {
        const TreeNode* node = stack.pop();
        sf::Vector2f from(390.f + viewX + (layout.xOf(node->id) - centre) * viewZoom,
                          210.f + viewY + layout.yOf(node->id) * viewZoom);
        if (node->left) {
            sf::Vector2f to(390.f + viewX + (layout.xOf(node->left->id) - centre) * viewZoom,
                            210.f + viewY + layout.yOf(node->left->id) * viewZoom);
            renderer.drawEdge(from, to, false); stack.push(node->left);
        }
        if (node->right) {
            sf::Vector2f to(390.f + viewX + (layout.xOf(node->right->id) - centre) * viewZoom,
                            210.f + viewY + layout.yOf(node->right->id) * viewZoom);
            renderer.drawEdge(from, to, false); stack.push(node->right);
        }
    }
    for (int i = 0; i < layout.size(); i++) {
        const LayoutPoint& point = layout.at(i);
        const TreeNode* node = logic->tree().findById(point.nodeId);
        renderer.drawNode({390.f + viewX + (point.x - centre) * viewZoom,
                           210.f + viewY + point.y * viewZoom},
                          node ? to_string(node->value) : "?", 0);
    }

    chrome.drawHud(window, renderer, theme, logic->score(), level.id, level.title, level.mission,
                   message, logic->isFailed());
    chrome.drawEventLog(renderer, lastEvents, lastEvents.size() - 1);

    chrome.drawButton(window, theme, renderer, LevelChrome::opButtonWide(), "Next Step [N]",
                      ButtonKind::PRIMARY);
    chrome.drawButton(window, theme, renderer, LevelChrome::undoButton(), "Undo [Z]");
    chrome.drawButton(window, theme, renderer, LevelChrome::hintButton(), "Hint [H]");
    chrome.drawButton(window, theme, renderer, LevelChrome::resetButton(), "Reset [R]",
                      ButtonKind::DANGER);
    chrome.drawButton(window, theme, renderer, LevelChrome::exitButton(), "Campaign [Esc]");
    if (session.showHelp) {
        chrome.drawHelp(renderer, "N next · H hint · Z undo · wheel zoom · drag pan · Esc map");
    }
}
}
