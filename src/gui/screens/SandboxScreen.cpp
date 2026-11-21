#include "gui/screens/SandboxScreen.h"
#include "game/tree/TreeLayout.h"
#include "gui/core/Renderer.h"
#include "gui/core/ScreenId.h"
#include "util/Rng.h"

namespace vh {

SandboxScreen::SandboxScreen(Theme& sharedTheme)
    : theme(sharedTheme), structure(1), inputValue(10), message("Sandbox ready."),
      heap(true) {
    animation.setListener(&heapView);
}

void SandboxScreen::onShow() {
    message = "Type digits for a value, then Insert. Structure switch cycles types.";
}

sf::FloatRect SandboxScreen::structureButton() const { return {{800.f, 60.f}, {260.f, 40.f}}; }
sf::FloatRect SandboxScreen::insertButton() const { return {{800.f, 160.f}, {120.f, 40.f}}; }
sf::FloatRect SandboxScreen::removeButton() const { return {{940.f, 160.f}, {120.f, 40.f}}; }
sf::FloatRect SandboxScreen::randomButton() const { return {{800.f, 220.f}, {120.f, 40.f}}; }
sf::FloatRect SandboxScreen::clearButton() const { return {{940.f, 220.f}, {120.f, 40.f}}; }
sf::FloatRect SandboxScreen::buildButton() const { return {{800.f, 280.f}, {260.f, 40.f}}; }
sf::FloatRect SandboxScreen::backButton() const { return {{800.f, 600.f}, {260.f, 40.f}}; }

string SandboxScreen::structureName() const {
    if (structure == 0) return "Binary Tree";
    if (structure == 1) return "BST";
    if (structure == 2) return "AVL";
    return "Max Heap";
}

void SandboxScreen::cycleStructure() {
    structure = (structure + 1) % 4;
    doClear();
    message = "Switched to " + structureName();
}

void SandboxScreen::refreshHeapView() {
    DynArray<int> values;
    for (int i = 0; i < heap.size(); i++) values.push(heap.at(i));
    heapView.load(values, heap.activeSize());
}

void SandboxScreen::doInsert() {
    if (structure == 3) {
        if (!heap.canInsert()) {
            message = "Heap is full.";
            return;
        }
        EventLog events = heap.insert(inputValue);
        refreshHeapView();
        animation.load(events);
        animation.play();
        message = "Inserted " + to_string(inputValue) + " into Max Heap.";
        return;
    }
    if (structure == 2) {
        BST::OpResult result = avl.insert(inputValue);
        message = result.ok ? ("AVL insert " + to_string(inputValue))
                            : (result.message.empty() ? "AVL insert failed" : result.message);
        return;
    }
    BST::OpResult result = bst.insert(inputValue);
    if (structure == 0) binary.deserialize(bst.serialize());
    message = result.ok ? ("Insert " + to_string(inputValue))
                        : (result.message.empty() ? "Insert failed" : result.message);
}

void SandboxScreen::doRemove() {
    if (structure == 3) {
        int maximum = 0;
        EventLog events = heap.extractMax(maximum);
        if (events.empty()) {
            message = "Heap is empty.";
            return;
        }
        refreshHeapView();
        animation.load(events);
        animation.play();
        message = "Extracted max " + to_string(maximum);
        return;
    }
    if (structure == 2) {
        BST::OpResult result = avl.deleteKey(inputValue);
        message = result.ok ? ("AVL delete " + to_string(inputValue))
                            : (result.message.empty() ? "Delete failed" : result.message);
        return;
    }
    BST::OpResult result = bst.deleteKey(inputValue);
    if (structure == 0) binary.deserialize(bst.serialize());
    message = result.ok ? ("Delete " + to_string(inputValue))
                        : (result.message.empty() ? "Delete failed" : result.message);
}

void SandboxScreen::doRandom() {
    Rng rng(static_cast<unsigned>(theme.timeSeconds() * 1000.f) + 17u);
    inputValue = rng.range(1, 99);
    doInsert();
}

void SandboxScreen::doClear() {
    binary.clear();
    bst.clear();
    avl.clear();
    DynArray<int> empty;
    heap.load(empty);
    refreshHeapView();
    EventLog blank;
    animation.load(blank);
    message = "Cleared " + structureName();
}

void SandboxScreen::doBuild() {
    if (structure != 3) {
        message = "Build Max Heap only applies to Max Heap mode.";
        return;
    }
    EventLog events = heap.buildMaxHeap();
    refreshHeapView();
    animation.load(events);
    animation.play();
    message = "buildMaxHeap complete.";
}

int SandboxScreen::handleEvent(const sf::Event& event, sf::RenderWindow& window) {
    if (const auto* release = event.getIf<sf::Event::MouseButtonReleased>()) {
        if (release->button == sf::Mouse::Button::Left) scrubber.release();
    }
    if (scrubber.isDragging()) scrubber.handleDrag(window, animation);

    const auto* key = event.getIf<sf::Event::KeyPressed>();
    if (key) {
        if (key->code == sf::Keyboard::Key::Escape) return ScreenId::MENU;
        if (key->code == sf::Keyboard::Key::Backspace) {
            inputValue /= 10;
            return ScreenId::NONE;
        }
        if (key->code >= sf::Keyboard::Key::Num0 && key->code <= sf::Keyboard::Key::Num9) {
            int digit = static_cast<int>(key->code) - static_cast<int>(sf::Keyboard::Key::Num0);
            inputValue = inputValue * 10 + digit;
            if (inputValue > 9999) inputValue = 9999;
            return ScreenId::NONE;
        }
        if (key->code == sf::Keyboard::Key::Enter) doInsert();
        else if (key->code == sf::Keyboard::Key::Space) {
            if (animation.isPlaying()) animation.pause();
            else animation.play();
        } else if (key->code == sf::Keyboard::Key::Left) animation.stepBackward();
        else if (key->code == sf::Keyboard::Key::Right) animation.stepForward();
        else if (key->code == sf::Keyboard::Key::P) {
            animation.restart();
            animation.play();
        }
    }

    const auto* click = event.getIf<sf::Event::MouseButtonPressed>();
    if (!click || click->button != sf::Mouse::Button::Left) return ScreenId::NONE;
    sf::Vector2f point = window.mapPixelToCoords(click->position);
    if (scrubber.handleClick(window, point, animation)) return ScreenId::NONE;
    if (structureButton().contains(point)) cycleStructure();
    else if (insertButton().contains(point)) doInsert();
    else if (removeButton().contains(point)) doRemove();
    else if (randomButton().contains(point)) doRandom();
    else if (clearButton().contains(point)) doClear();
    else if (buildButton().contains(point)) doBuild();
    else if (backButton().contains(point)) return ScreenId::MENU;
    return ScreenId::NONE;
}

int SandboxScreen::update(float seconds) {
    animation.update(seconds);
    return ScreenId::NONE;
}

void SandboxScreen::draw(sf::RenderWindow& window) {
    theme.drawBackdrop(window);
    Renderer renderer(window, theme.getFont());
    theme.drawPanel(window, {{28.f, 28.f}, {740.f, 600.f}});
    theme.drawPanel(window, {{788.f, 28.f}, {284.f, 640.f}});

    renderer.drawText("SANDBOX", {804.f, 40.f}, 14, Palette::MUTED, false);
    theme.drawButton(window, theme.getFont(), structureButton(), structureName(),
                     ButtonKind::PRIMARY, theme.hovered(window, structureButton()));
    renderer.drawText("Value  " + to_string(inputValue), {804.f, 120.f}, 18, Palette::BRASS, false);

    theme.drawButton(window, theme.getFont(), insertButton(), "Insert", ButtonKind::SECONDARY,
                     theme.hovered(window, insertButton()));
    theme.drawButton(window, theme.getFont(), removeButton(),
                     structure == 3 ? "Extract" : "Delete", ButtonKind::SECONDARY,
                     theme.hovered(window, removeButton()));
    theme.drawButton(window, theme.getFont(), randomButton(), "Random", ButtonKind::GHOST,
                     theme.hovered(window, randomButton()));
    theme.drawButton(window, theme.getFont(), clearButton(), "Clear", ButtonKind::DANGER,
                     theme.hovered(window, clearButton()));
    theme.drawButton(window, theme.getFont(), buildButton(), "Build Max Heap", ButtonKind::SECONDARY,
                     theme.hovered(window, buildButton()));
    theme.drawButton(window, theme.getFont(), backButton(), "Main Menu", ButtonKind::GHOST,
                     theme.hovered(window, backButton()));

    renderer.drawText(message, {804.f, 340.f}, 13, Palette::WARNING, false);
    renderer.drawText("Digits set value · Enter inserts", {804.f, 400.f}, 12, Palette::MUTED, false);

    if (structure == 3) {
        heapView.draw(renderer, {60.f, 70.f}, 680.f, {60.f, 480.f}, 680.f);
        scrubber.draw(window, renderer, theme, animation);
    } else {
        const BinaryTree* tree = &bst;
        if (structure == 0) tree = &binary;
        if (structure == 2) tree = &avl;
        if (structure == 1) tree = &bst;

        if (tree->root() == nullptr) {
            renderer.drawText("Empty tree", {400.f, 300.f}, 20, Palette::MUTED, true);
        } else {
            TreeLayout layout = TreeLayout::compute(*tree, 70.f, 82.f);
            float centre = (layout.bounds().minX + layout.bounds().maxX) / 2.f;
            DynArray<const TreeNode*> stack;
            stack.push(tree->root());
            while (!stack.empty()) {
                const TreeNode* node = stack.pop();
                sf::Vector2f from(390.f + (layout.xOf(node->id) - centre),
                                  160.f + layout.yOf(node->id));
                if (node->left) {
                    sf::Vector2f to(390.f + (layout.xOf(node->left->id) - centre),
                                    160.f + layout.yOf(node->left->id));
                    renderer.drawEdge(from, to, false);
                    stack.push(node->left);
                }
                if (node->right) {
                    sf::Vector2f to(390.f + (layout.xOf(node->right->id) - centre),
                                    160.f + layout.yOf(node->right->id));
                    renderer.drawEdge(from, to, false);
                    stack.push(node->right);
                }
            }
            for (int i = 0; i < layout.size(); i++) {
                const LayoutPoint& point = layout.at(i);
                const TreeNode* node = tree->findById(point.nodeId);
                renderer.drawNode({390.f + (point.x - centre), 160.f + point.y},
                                  node ? to_string(node->value) : "?", 0);
            }
        }
    }
}

}
