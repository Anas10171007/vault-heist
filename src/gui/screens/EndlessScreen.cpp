#include "gui/screens/EndlessScreen.h"

#include "game/Leaderboard.h"
#include "gui/core/Renderer.h"
#include "gui/core/ScreenId.h"
#include "model/tree/BinaryTree.h"
#include "model/tree/BST.h"
#include "model/tree/TreeValidator.h"

using namespace std;

namespace vh {

namespace {
constexpr int MAX_OPS = 25;
}

EndlessScreen::EndlessScreen(Theme& sharedTheme)
    : theme(sharedTheme), avl(false), rng(0xE11DCAFEu) {
    view.setModel(avl);
    view.setOrigin(60.f, 120.f);
}

void EndlessScreen::onShow() {
    avl.clear();
    for (int i = 0; i < 6; i++) {
        avl.insert(rng.range(20, 80));
    }
    view.setModel(avl);
    refreshView();
    elapsed = 0.f;
    ops = 0;
    typedValue = 0;
    ended = false;
    postedScore = false;
    lastScore = 0;
    pendingHas = false;
    nextOpTimer = delay();
}

float EndlessScreen::delay() const {
    float interval = 1.6f - 0.03f * static_cast<float>(elapsed);
    if (interval < 0.5f) interval = 0.5f;
    return interval;
}

void EndlessScreen::generateOp() {
    EndlessOp op;
    BinaryTree::TraversalResult walk = avl.inorder();
    DynArray<int> values;
    for (int i = 0; i < walk.ids.size(); i++) {
        TreeNode* node = avl.findById(walk.ids[i]);
        if (node != nullptr) values.push(node->value);
    }
    if (values.size() > 0 && rng.range(0, 99) < 45) {
        op.insert = false;
        op.value = values[rng.range(0, values.size() - 1)];
    } else {
        op.insert = true;
        op.value = rng.range(20, 80);
    }
    pending = op;
    pendingHas = true;
}

bool EndlessScreen::isValid() const {
    return TreeValidator::validateTree(avl).ok();
}

void EndlessScreen::finishRun() {
    if (ended) return;
    ended = true;
    pendingHas = false;
    lastScore = static_cast<int>(elapsed + 0.5f);
    scores.push(lastScore);
    if (!postedScore) {
        Leaderboard board;
        board.load();
        board.addScore("Endless", lastScore);
        board.save();
        postedScore = true;
    }
}

void EndlessScreen::applyPending() {
    if (!pendingHas) return;
    pendingHas = false;
    if (pending.insert) {
        avl.insert(pending.value);
    } else {
        TreeNode* node = avl.findByValue(pending.value);
        if (node != nullptr) avl.deleteKey(pending.value);
    }
    ops++;
    refreshView();
    if (!isValid() || ops >= MAX_OPS) finishRun();
}

void EndlessScreen::refreshView() {
    TreeLayout layout = TreeLayout::compute(avl);
    view.setMotion(TreeMotion::simple(layout, EventLog(), layout));
}

int EndlessScreen::handleEvent(const sf::Event& event, sf::RenderWindow& window) {
    const auto* key = event.getIf<sf::Event::KeyPressed>();
    if (key != nullptr) {
        if (key->code == sf::Keyboard::Key::Escape && ended) return ScreenId::MENU;
        if (ended) return ScreenId::NONE;
        if (key->code >= sf::Keyboard::Key::Num0 && key->code <= sf::Keyboard::Key::Num9) {
            typedValue = typedValue * 10 + static_cast<int>(key->code) -
                         static_cast<int>(sf::Keyboard::Key::Num0);
            if (typedValue > 90) typedValue = 90;
            return ScreenId::NONE;
        }
        if (key->code == sf::Keyboard::Key::Enter) {
            if (typedValue >= 20) {
                avl.insert(typedValue);
                refreshView();
                if (!isValid()) finishRun();
            }
            typedValue = 0;
            return ScreenId::NONE;
        }
        if (key->code == sf::Keyboard::Key::Backspace) {
            typedValue /= 10;
            return ScreenId::NONE;
        }
        return ScreenId::NONE;
    }

    if (!ended) return ScreenId::NONE;

    const auto* click = event.getIf<sf::Event::MouseButtonPressed>();
    if (click == nullptr || click->button != sf::Mouse::Button::Left) return ScreenId::NONE;
    sf::Vector2f point = window.mapPixelToCoords(click->position);
    if (contains(menuButton(), point)) return ScreenId::MENU;
    if (contains(restartButton(), point)) return ScreenId::ENDLESS;
    return ScreenId::NONE;
}

int EndlessScreen::update(float deltaSeconds) {
    if (ended) return ScreenId::NONE;
    elapsed += deltaSeconds;
    nextOpTimer -= deltaSeconds;
    if (nextOpTimer <= 0.f) {
        if (!pendingHas) generateOp();
        applyPending();
        nextOpTimer = delay();
    }
    return ScreenId::NONE;
}

void EndlessScreen::draw(sf::RenderWindow& window) {
    theme.drawBackdrop(window);
    Renderer renderer(window, theme.getFont());
    view.draw(renderer);
    renderer.drawText("ENDLESS MODE", {550.f, 130.f}, 30, Palette::TARGET, true);
    renderer.drawText("Mixed inserts and deletes; the tree must stay valid",
                      {550.f, 170.f}, 15, Palette::MUTED, true);

    if (!ended) {
        string typed = typedValue == 0 ? "-" : to_string(typedValue);
        renderer.drawText("SURVIVAL " + to_string(static_cast<int>(elapsed + 0.5f)) +
                              "s   OPS " + to_string(ops) + "   TYPE " + typed +
                              "  [Enter inserts]",
                          {550.f, 660.f}, 16, Palette::TEXT, true);
        return;
    }

    renderer.drawText("RUN OVER", {550.f, 360.f}, 30, Palette::TARGET, true);
    renderer.drawText("SURVIVAL " + to_string(lastScore) + "s   OPS " + to_string(ops),
                      {550.f, 410.f}, 20, Palette::TEXT, true);
    renderer.drawText("Score saved to the local leaderboard as Endless.",
                      {550.f, 450.f}, 14, Palette::MUTED, true);

    sf::FloatRect menu = menuButton();
    sf::FloatRect restart = restartButton();
    theme.drawButton(window, theme.getFont(), menu, "Main Menu", ButtonKind::PRIMARY,
                     theme.hovered(window, menu));
    theme.drawButton(window, theme.getFont(), restart, "Try Endless Again",
                     ButtonKind::SECONDARY, theme.hovered(window, restart));
}

bool EndlessScreen::contains(sf::FloatRect bounds, sf::Vector2f point) {
    return bounds.contains(point);
}

sf::FloatRect EndlessScreen::menuButton() { return {{375.f, 520.f}, {350.f, 44.f}}; }
sf::FloatRect EndlessScreen::restartButton() { return {{375.f, 580.f}, {350.f, 44.f}}; }

}
