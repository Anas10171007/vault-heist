#include "gui/screens/SandboxScreen.h"
#include "game/tree/TreeLayout.h"
#include "gui/core/Renderer.h"
#include "gui/core/ScreenId.h"
#include "util/Rng.h"

namespace vh {

SandboxScreen::SandboxScreen(Theme& sharedTheme)
    : theme(sharedTheme), structure(1), inputValue(10),
      selectedNodeId(-1), message("Sandbox ready."), searchStep(0), searchClock(0.f),
      heap(true) {}

void SandboxScreen::onShow() {
    message = "Click a field, type a number, Enter inserts.";
}

sf::FloatRect SandboxScreen::structureButton() const { return {{800.f, 60.f}, {260.f, 34.f}}; }
sf::FloatRect SandboxScreen::valueField() const { return {{800.f, 96.f}, {260.f, 36.f}}; }
sf::FloatRect SandboxScreen::insertButton() const { return {{800.f, 148.f}, {120.f, 32.f}}; }
sf::FloatRect SandboxScreen::removeButton() const { return {{940.f, 148.f}, {120.f, 32.f}}; }
sf::FloatRect SandboxScreen::searchButton() const { return {{800.f, 188.f}, {120.f, 32.f}}; }
sf::FloatRect SandboxScreen::rangeButton() const { return {{940.f, 188.f}, {120.f, 32.f}}; }
sf::FloatRect SandboxScreen::rotateLButton() const { return {{800.f, 228.f}, {120.f, 32.f}}; }
sf::FloatRect SandboxScreen::rotateRButton() const { return {{940.f, 228.f}, {120.f, 32.f}}; }
sf::FloatRect SandboxScreen::randomButton() const { return {{800.f, 268.f}, {120.f, 32.f}}; }
sf::FloatRect SandboxScreen::clearButton() const { return {{940.f, 268.f}, {120.f, 32.f}}; }
sf::FloatRect SandboxScreen::buildButton() const { return {{800.f, 308.f}, {260.f, 32.f}}; }
sf::FloatRect SandboxScreen::backButton() const { return {{800.f, 620.f}, {260.f, 36.f}}; }

string SandboxScreen::structureName() const {
    if (structure == 0) return "Binary Tree";
    if (structure == 1) return "BST";
    if (structure == 2) return "AVL";
    return "Max Heap";
}

void SandboxScreen::cycleStructure() {
    structure = (structure + 1) % 4;
    selectedNodeId = -1;
    doClear();
    message = "Switched to " + structureName();
}

void SandboxScreen::clearSearch() {
    searchLog.clear();
    searchStep = 0;
    searchClock = 0.f;
}

void SandboxScreen::refreshHeapView() {
    // Snapshot the finished heap. Replay used to reset the view to empty.
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
        heap.insert(inputValue);
        refreshHeapView();
        message = "Inserted " + to_string(inputValue) + ". Type a new number, then Insert.";
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

void SandboxScreen::doSearch() {
    if (structure == 3) {
        message = "Search is for trees. Use heap extract for Max Heap.";
        return;
    }
    if (structure == 0) {
        message = "Search needs BST or AVL ordering.";
        return;
    }
    BST::SearchResult found = structure == 2 ? avl.search(inputValue) : bst.search(inputValue);
    searchLog.clear();
    searchLog.extend(found.events);
    searchStep = 0;
    searchClock = 0.f;
    if (searchLog.size() == 0) {
        message = "Tree is empty.";
        return;
    }
    message = found.found != nullptr
                  ? ("Searching " + to_string(inputValue) + " — watch the path")
                  : ("Searching " + to_string(inputValue) + " — not in the tree");
}

void SandboxScreen::doRange() {
    if (structure != 1 && structure != 2) {
        message = "Range query needs BST or AVL.";
        return;
    }
    // Brief asks for range(lo, hi). One typed value is the low end; hi is the top of the key space.
    int lo = inputValue;
    int hi = 99;
    if (structure == 2) {
        BST::RangeResult result = avl.rangeQuery(lo, hi);
        message = "AVL range [" + to_string(lo) + "," + to_string(hi) + "] → " +
                  to_string(result.keys.size()) + " keys";
        return;
    }
    BST::RangeResult result = bst.rangeQuery(lo, hi);
    message = "BST range [" + to_string(lo) + "," + to_string(hi) + "] → " +
              to_string(result.keys.size()) + " keys";
}

void SandboxScreen::doRotate(bool left) {
    if (structure != 2) {
        message = "Rotate is AVL-only. Switch structure.";
        return;
    }
    if (selectedNodeId < 0) {
        message = "Click an AVL node, then Rotate L/R.";
        return;
    }
    BST::OpResult result = left ? avl.rotateLeft(selectedNodeId) : avl.rotateRight(selectedNodeId);
    message = result.ok ? (left ? "Rotated left." : "Rotated right.")
                        : (result.message.empty() ? "Rotate failed" : result.message);
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
    clearSearch();
    selectedNodeId = -1;
    message = "Cleared " + structureName();
}

void SandboxScreen::doBuild() {
    if (structure != 3) {
        message = "Build Max Heap only in Max Heap mode.";
        return;
    }
    if (heap.size() == 0) {
        message = "Insert some numbers first, then Build Max Heap.";
        return;
    }
    // Insert already sifts up, so the array is a heap. Rebuild from the raw
    // values so this button actually reorders something you can see.
    DynArray<int> raw;
    for (int i = 0; i < heap.size(); i++) raw.push(heap.at(i));
    heap.load(raw);
    heap.buildMaxHeap();
    refreshHeapView();
    message = "Rebuilt Max Heap from " + to_string(raw.size()) + " values.";
}

int SandboxScreen::handleEvent(const sf::Event& event, sf::RenderWindow& window) {
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
    }

    const auto* click = event.getIf<sf::Event::MouseButtonPressed>();
    if (!click || click->button != sf::Mouse::Button::Left) return ScreenId::NONE;
    sf::Vector2f point = window.mapPixelToCoords(click->position);

    // Tree node pick for AVL rotate (canvas left panel).
    if (structure == 2 && point.x < 780.f && point.y < 600.f) {
        const BinaryTree* tree = &avl;
        if (tree->root() != nullptr) {
            TreeLayout layout = TreeLayout::compute(*tree, 56.f, 70.f);
            float spanX = layout.bounds().maxX - layout.bounds().minX;
            float spanY = layout.bounds().maxY - layout.bounds().minY;
            float scale = 1.f;
            if (spanX > 640.f) scale = 640.f / spanX;
            if (spanY > 420.f) {
                float ys = 420.f / spanY;
                if (ys < scale) scale = ys;
            }
            float centre = (layout.bounds().minX + layout.bounds().maxX) / 2.f;
            for (int i = 0; i < layout.size(); i++) {
                const LayoutPoint& lp = layout.at(i);
                sf::Vector2f pos(390.f + (lp.x - centre) * scale,
                                 120.f + lp.y * scale);
                if ((point - pos).length() <= 22.f) {
                    selectedNodeId = lp.nodeId;
                    message = "Selected AVL node id " + to_string(selectedNodeId);
                    return ScreenId::NONE;
                }
            }
        }
    }

    if (valueField().contains(point)) {
        message = "Type digits, Enter inserts.";
        return ScreenId::NONE;
    }
    if (structureButton().contains(point)) cycleStructure();
    else if (insertButton().contains(point)) doInsert();
    else if (removeButton().contains(point)) doRemove();
    else if (searchButton().contains(point)) doSearch();
    else if (rangeButton().contains(point)) doRange();
    else if (rotateLButton().contains(point)) doRotate(true);
    else if (rotateRButton().contains(point)) doRotate(false);
    else if (randomButton().contains(point)) doRandom();
    else if (clearButton().contains(point)) doClear();
    else if (buildButton().contains(point)) doBuild();
    else if (backButton().contains(point)) return ScreenId::MENU;
    return ScreenId::NONE;
}

int SandboxScreen::update(float seconds) {
    if (searchLog.size() == 0 || searchStep >= searchLog.size()) return ScreenId::NONE;
    searchClock += seconds;
    while (searchClock >= 0.45f && searchStep < searchLog.size()) {
        searchClock -= 0.45f;
        searchStep++;
    }
    if (searchStep >= searchLog.size()) {
        bool found = false;
        for (int i = 0; i < searchLog.size(); i++) {
            if (searchLog.at(i).type == EventType::COMPARE && searchLog.at(i).extra == 0) found = true;
        }
        message = found ? ("Found " + to_string(inputValue))
                        : ("Not found " + to_string(inputValue));
    }
    return ScreenId::NONE;
}

int SandboxScreen::searchState(int nodeId) const {
    int state = NodeState::NORMAL;
    int limit = searchStep;
    if (limit > searchLog.size()) limit = searchLog.size();
    int currentId = -1;
    for (int i = 0; i < limit; i++) {
        const Event& event = searchLog.at(i);
        if (event.type != EventType::COMPARE) continue;
        currentId = event.idA;
    }
    for (int i = 0; i < limit; i++) {
        const Event& event = searchLog.at(i);
        if (event.type != EventType::COMPARE || event.idA != nodeId) continue;
        if (event.idA == currentId && event.extra == 0) state = NodeState::TARGET;
        else if (event.idA == currentId) state = NodeState::WARNING;
        else state = NodeState::VISITED;
    }
    return state;
}

void SandboxScreen::drawTree(Renderer& renderer) {
    const BinaryTree* tree = &bst;
    if (structure == 0) tree = &binary;
    if (structure == 2) tree = &avl;
    if (structure == 1) tree = &bst;

    if (tree->root() == nullptr) {
        renderer.drawText("Empty tree", {400.f, 280.f}, 18, Palette::MUTED, true);
        return;
    }

    TreeLayout layout = TreeLayout::compute(*tree, 56.f, 70.f);
    float spanX = layout.bounds().maxX - layout.bounds().minX;
    float spanY = layout.bounds().maxY - layout.bounds().minY;
    float scale = 1.f;
    if (spanX > 640.f) scale = 640.f / spanX;
    if (spanY > 420.f) {
        float ys = 420.f / spanY;
        if (ys < scale) scale = ys;
    }
    float centre = (layout.bounds().minX + layout.bounds().maxX) / 2.f;

    DynArray<const TreeNode*> stack;
    stack.push(tree->root());
    while (!stack.empty()) {
        const TreeNode* node = stack.pop();
        sf::Vector2f from(390.f + (layout.xOf(node->id) - centre) * scale,
                          120.f + layout.yOf(node->id) * scale);
        if (node->left) {
            sf::Vector2f to(390.f + (layout.xOf(node->left->id) - centre) * scale,
                            120.f + layout.yOf(node->left->id) * scale);
            renderer.drawEdge(from, to, false);
            stack.push(node->left);
        }
        if (node->right) {
            sf::Vector2f to(390.f + (layout.xOf(node->right->id) - centre) * scale,
                            120.f + layout.yOf(node->right->id) * scale);
            renderer.drawEdge(from, to, false);
            stack.push(node->right);
        }
    }
    for (int i = 0; i < layout.size(); i++) {
        const LayoutPoint& point = layout.at(i);
        const TreeNode* node = tree->findById(point.nodeId);
        int state = searchState(point.nodeId);
        if (state == NodeState::NORMAL && structure == 2 && point.nodeId == selectedNodeId) {
            state = NodeState::TARGET;
        }
        renderer.drawNode({390.f + (point.x - centre) * scale, 120.f + point.y * scale},
                          node ? to_string(node->value) : "?", state);
    }
}

void SandboxScreen::draw(sf::RenderWindow& window) {
    theme.drawBackdrop(window);
    Renderer renderer(window, theme.getFont());
    theme.drawPanel(window, {{28.f, 28.f}, {740.f, 560.f}});
    theme.drawPanel(window, {{788.f, 28.f}, {284.f, 640.f}});

    bool heapMode = structure == 3;
    bool ordered = structure == 1 || structure == 2;
    bool avlMode = structure == 2;

    renderer.drawText("SANDBOX", {804.f, 36.f}, 13, Palette::MUTED, false);
    theme.drawButton(window, theme.getFont(), structureButton(), structureName(),
                     ButtonKind::PRIMARY, theme.hovered(window, structureButton()));

    auto drawField = [&](sf::FloatRect bounds, const string& label, int value, bool active) {
        sf::RectangleShape box(bounds.size);
        box.setPosition(bounds.position);
        box.setFillColor(active ? sf::Color(40, 48, 36) : sf::Color(24, 28, 32));
        box.setOutlineColor(active ? Palette::BRASS : Palette::BORDER);
        box.setOutlineThickness(2.f);
        window.draw(box);
        renderer.drawText(label, {bounds.position.x + 8.f, bounds.position.y + 2.f}, 10,
                          Palette::MUTED, false);
        renderer.drawText(to_string(value), {bounds.position.x + 8.f, bounds.position.y + 16.f}, 14,
                          active ? Palette::BRASS : Palette::TEXT, false);
    };
    drawField(valueField(), "VALUE", inputValue, true);

    theme.drawButton(window, theme.getFont(), insertButton(), "Insert", ButtonKind::SECONDARY,
                     theme.hovered(window, insertButton()));
    theme.drawButton(window, theme.getFont(), removeButton(),
                     heapMode ? "Extract" : "Delete", ButtonKind::SECONDARY,
                     theme.hovered(window, removeButton()));
    if (ordered) {
        theme.drawButton(window, theme.getFont(), searchButton(), "Search", ButtonKind::GHOST,
                         theme.hovered(window, searchButton()));
        theme.drawButton(window, theme.getFont(), rangeButton(), "Range", ButtonKind::GHOST,
                         theme.hovered(window, rangeButton()));
    }
    if (avlMode) {
        theme.drawButton(window, theme.getFont(), rotateLButton(), "Rot L", ButtonKind::GHOST,
                         theme.hovered(window, rotateLButton()));
        theme.drawButton(window, theme.getFont(), rotateRButton(), "Rot R", ButtonKind::GHOST,
                         theme.hovered(window, rotateRButton()));
    }
    theme.drawButton(window, theme.getFont(), randomButton(), "Random", ButtonKind::GHOST,
                     theme.hovered(window, randomButton()));
    theme.drawButton(window, theme.getFont(), clearButton(), "Clear", ButtonKind::DANGER,
                     theme.hovered(window, clearButton()));
    if (heapMode) {
        theme.drawButton(window, theme.getFont(), buildButton(), "Build Max Heap",
                         ButtonKind::SECONDARY, theme.hovered(window, buildButton()));
    }
    theme.drawButton(window, theme.getFont(), backButton(), "Main Menu", ButtonKind::GHOST,
                     theme.hovered(window, backButton()));

    string msg = message;
    if (msg.size() > 42) msg = msg.substr(0, 41) + "…";
    renderer.drawText(msg, {804.f, 360.f}, 12, Palette::WARNING, false);
    renderer.drawText("Type a number, Enter inserts it.",
                      {804.f, 400.f}, 11, Palette::MUTED, false);

    if (heapMode) {
        heapView.draw(renderer, {48.f, 56.f}, 700.f, {48.f, 430.f}, 700.f);
    } else {
        drawTree(renderer);
    }
}

}
