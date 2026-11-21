#include "gui/screens/HeapLevelScreen.h"
#include "game/LevelManager.h"
#include "gui/core/Renderer.h"
#include "gui/core/ScreenId.h"

namespace vh {

HeapLevelScreen::HeapLevelScreen(Theme& sharedTheme)
    : theme(sharedTheme), levelLoaded(false), firstSelection(-1),
      waitingForAnimation(false) {
    animation.setListener(&heapView);
    loadLevel("4.1");
}

bool HeapLevelScreen::loadLevel(const string& id) {
    const char* folders[4] = {"data/levels", "../data/levels",
                              "../../data/levels", "../../../data/levels"};
    levelLoaded = false;
    for (int index = 0; index < 4; index++) {
        LevelManager manager;
        if (!manager.loadAll(folders[index])) continue;
        const LevelDef* found = manager.load(id);
        if (found != nullptr) {
            level = *found;
            levelLoaded = true;
            resetLevel();
            return true;
        }
    }
    message = "Could not load Act 4 level data.";
    return false;
}

bool HeapLevelScreen::selectLevel(const string& id) { return loadLevel(id); }
const ScoreSystem& HeapLevelScreen::score() const { return currentScore(); }
void HeapLevelScreen::setAnimationSpeed(float speed) { animation.setSpeed(speed); }

bool HeapLevelScreen::isBuildLevel() const {
    return level.task == "heap_build";
}

bool HeapLevelScreen::isRepairLevel() const {
    return level.task == "heap_repair";
}

bool HeapLevelScreen::isOperationLevel() const {
    return level.task == "heap_extract" || level.task == "heap_race" ||
           level.task == "heap_sort";
}

bool HeapLevelScreen::currentComplete() const {
    if (isBuildLevel()) return buildLogic.isComplete();
    if (isRepairLevel()) return repairLogic.isComplete();
    return operationLogic.isComplete();
}

bool HeapLevelScreen::currentFailed() const {
    if (isBuildLevel()) return buildLogic.isFailed();
    if (isRepairLevel()) return repairLogic.isFailed();
    return operationLogic.isFailed();
}

const ScoreSystem& HeapLevelScreen::currentScore() const {
    if (isBuildLevel()) return buildLogic.score();
    if (isRepairLevel()) return repairLogic.score();
    return operationLogic.score();
}

void HeapLevelScreen::copyCurrentValues(DynArray<int>& output) const {
    if (isBuildLevel()) buildLogic.copyValues(output);
    else if (isRepairLevel()) repairLogic.copyValues(output);
    else operationLogic.copyValues(output);
}

void HeapLevelScreen::resetLevel() {
    firstSelection = -1;
    waitingForAnimation = false;
    EventLog empty;
    animation.load(empty);
    if (!levelLoaded) return;

    if (isBuildLevel()) {
        buildLogic.load(level.data.values);
        message = "Select a smaller parent, then its largest child.";
    } else if (isRepairLevel()) {
        repairLogic.load(level.data.values);
        message = "Select a BAD node, then choose a heapify direction.";
    } else {
        operationLogic.load(level.data.values, level.task);
        message = level.task == "heap_sort" ? "Run Heap Sort to secure the vault."
                                             : "Extract the current maximum.";
    }
    DynArray<int> values;
    copyCurrentValues(values);
    int active = isOperationLevel() ? operationLogic.activeSize() : values.size();
    heapView.load(values, active);
}

void HeapLevelScreen::onShow() {
    resetLevel();
}

void HeapLevelScreen::selectIndex(int index) {
    if (!isBuildLevel()) {
        if (index == firstSelection) {
            firstSelection = -1;
            heapView.setSelectedIndex(-1);
            message = "Selection cleared.";
        } else {
            firstSelection = index;
            heapView.setSelectedIndex(index);
            message = "Choose Heapify Up or Heapify Down.";
        }
        return;
    }

    if (firstSelection < 0) {
        firstSelection = index;
        heapView.setSelectedIndex(index);
        message = "Now select the connected larger child.";
        return;
    }

    if (index == firstSelection) {
        firstSelection = -1;
        heapView.setSelectedIndex(-1);
        message = "Selection cleared.";
        return;
    }

    DynArray<int> before;
    buildLogic.copyValues(before);
    bool accepted = false;
    EventLog events = buildLogic.chooseSwap(firstSelection, index, accepted);
    firstSelection = -1;
    heapView.setSelectedIndex(-1);

    if (!accepted) {
        message = buildLogic.isFailed() ? "Alarm full. Reset to try again."
                                        : "Invalid parent-child swap.";
        return;
    }

    heapView.load(before, before.size());
    animation.load(events);
    animation.play();
    waitingForAnimation = true;
    message = buildLogic.isComplete() ? "Final valid swap..." : "Valid swap.";
}

void HeapLevelScreen::chooseDirection(int direction) {
    if (firstSelection < 0) {
        message = "Select a heap node first.";
        return;
    }

    DynArray<int> before;
    repairLogic.copyValues(before);
    bool accepted = false;
    EventLog events = repairLogic.chooseRepair(firstSelection, direction, accepted);
    firstSelection = -1;
    heapView.setSelectedIndex(-1);

    if (!accepted) {
        message = repairLogic.isFailed() ? "Alarm full. Reset to try again."
                                         : "That node does not need this direction.";
        return;
    }

    heapView.load(before, before.size());
    animation.load(events);
    animation.play();
    waitingForAnimation = true;
    message = repairLogic.isComplete() ? "Repairing the final violation..."
                                       : "Valid repair. Check the remaining BAD nodes.";
}

void HeapLevelScreen::performOperation() {
    DynArray<int> before;
    operationLogic.copyValues(before);
    EventLog events = operationLogic.perform();
    if (events.empty()) return;
    heapView.load(before, operationLogic.activeSize() +
                          (level.task == "heap_sort" ? before.size() : 1));
    animation.load(events);
    animation.play();
    waitingForAnimation = true;
    message = level.task == "heap_sort" ? "Heap Sort running..." : "Extracting maximum...";
}

void HeapLevelScreen::undoOperation() {
    if (!isOperationLevel() || waitingForAnimation || !operationLogic.undo()) {
        message = "Nothing to undo.";
        return;
    }
    DynArray<int> restored;
    operationLogic.copyValues(restored);
    heapView.load(restored, operationLogic.activeSize());
    message = "Last operation undone.";
}

int HeapLevelScreen::handleEvent(const sf::Event& event, sf::RenderWindow& window) {
    const sf::Event::KeyPressed* key = event.getIf<sf::Event::KeyPressed>();
    if (key != nullptr) {
        if (key->code == sf::Keyboard::Key::Num1) loadLevel("4.1");
        else if (key->code == sf::Keyboard::Key::Num2) loadLevel("4.2");
        else if (key->code == sf::Keyboard::Key::Num3) loadLevel("4.3");
        else if (key->code == sf::Keyboard::Key::Num4) loadLevel("4.4");
        else if (key->code == sf::Keyboard::Key::Num5) loadLevel("4.5");
        else if (key->code == sf::Keyboard::Key::R) resetLevel();
        else if (key->code == sf::Keyboard::Key::Z) undoOperation();
        else if (key->code == sf::Keyboard::Key::P) { animation.restart(); animation.play(); }
        else if (key->code == sf::Keyboard::Key::Space) {
            if (animation.isPlaying()) animation.pause();
            else animation.play();
        } else if (key->code == sf::Keyboard::Key::Right) animation.stepForward();
        else if (key->code == sf::Keyboard::Key::Left) animation.stepBackward();
        else if (key->code == sf::Keyboard::Key::Enter && currentComplete()) {
            return ScreenId::RESULT;
        }
    }

    const sf::Event::MouseButtonPressed* click = event.getIf<sf::Event::MouseButtonPressed>();
    if (click == nullptr || click->button != sf::Mouse::Button::Left) {
        return ScreenId::NONE;
    }

    sf::Vector2f point = window.mapPixelToCoords(click->position);
    if (resetButton().contains(point)) {
        resetLevel();
        return ScreenId::NONE;
    }
    if (isOperationLevel() && undoButton().contains(point)) {
        undoOperation();
        return ScreenId::NONE;
    }
    if (menuButton().contains(point)) return ScreenId::MENU;
    if (levelLoaded && currentComplete() && continueButton().contains(point)) {
        return ScreenId::RESULT;
    }

    bool replayAtEnd = animation.completedCount() == animation.eventCount();
    bool canAct = levelLoaded && !currentComplete() && !currentFailed() &&
                  !waitingForAnimation && replayAtEnd;
    if (isOperationLevel() && canAct && actionButton().contains(point)) {
        performOperation();
        return ScreenId::NONE;
    }
    if (isRepairLevel() && canAct && upButton().contains(point)) {
        chooseDirection(HeapDirection::UP);
        return ScreenId::NONE;
    }
    if (isRepairLevel() && canAct && downButton().contains(point)) {
        chooseDirection(HeapDirection::DOWN);
        return ScreenId::NONE;
    }
    if (!canAct) return ScreenId::NONE;

    int index = heapView.hitTest(point, {40.f, 105.f}, 750.f,
                                 {40.f, 520.f}, 750.f);
    if (index >= 0) selectIndex(index);
    return ScreenId::NONE;
}

void HeapLevelScreen::update(float seconds) {
    animation.update(seconds);
    if (waitingForAnimation && !animation.isPlaying() &&
        animation.completedCount() == animation.eventCount()) {
        waitingForAnimation = false;
        if (currentComplete()) {
            if (isBuildLevel()) message = "Vault activated. Max Heap complete.";
            else if (isRepairLevel()) message = "Security locks repaired. Max Heap restored.";
            else message = "Vault cleared. Sorted sequence complete.";
        } else {
            message = isBuildLevel() ? "Valid swap. Repair the next violation."
                                     : "Valid repair. Select the next BAD node.";
        }
    }
}

sf::FloatRect HeapLevelScreen::upButton() const {
    return {{835.f, 448.f}, {105.f, 38.f}};
}

sf::FloatRect HeapLevelScreen::downButton() const {
    return {{950.f, 448.f}, {105.f, 38.f}};
}

sf::FloatRect HeapLevelScreen::actionButton() const {
    return {{835.f, 448.f}, {220.f, 38.f}};
}

sf::FloatRect HeapLevelScreen::undoButton() const {
    return {{835.f, 400.f}, {220.f, 38.f}};
}

sf::FloatRect HeapLevelScreen::resetButton() const {
    return {{840.f, 500.f}, {210.f, 38.f}};
}

sf::FloatRect HeapLevelScreen::continueButton() const {
    return {{840.f, 548.f}, {210.f, 38.f}};
}

sf::FloatRect HeapLevelScreen::menuButton() const {
    return {{840.f, 596.f}, {210.f, 38.f}};
}

void HeapLevelScreen::drawButton(sf::RenderWindow& window, Renderer& renderer,
                                 sf::FloatRect bounds, const string& label) const {
    sf::RectangleShape button = theme.makeButton(bounds.position, bounds.size);
    window.draw(button);
    renderer.drawText(label, bounds.position + bounds.size / 2.f,
                      16, Palette::TEXT, true);
}

void HeapLevelScreen::draw(sf::RenderWindow& window) {
    Renderer renderer(window, theme.getFont());
    sf::RectangleShape panel = theme.makePanel({815.f, 45.f}, {260.f, 620.f});
    window.draw(panel);

    if (levelLoaded) {
        heapView.draw(renderer, {40.f, 105.f}, 750.f, {40.f, 520.f}, 750.f);
        renderer.drawText(level.id, {835.f, 68.f}, 18, Palette::TARGET, false);
        renderer.drawText(level.title, {835.f, 96.f}, 16, Palette::TEXT, false);
        renderer.drawText("Levels: 1 Build  2 Repair  3-5 Ops", {835.f, 126.f},
                          12, Palette::MUTED, false);

        if (isBuildLevel()) {
            renderer.drawText("Build a Max Heap manually.", {835.f, 158.f},
                              15, Palette::TEXT, false);
            renderer.drawText("1. Select a smaller parent", {835.f, 188.f},
                              13, Palette::TEXT, false);
            renderer.drawText("2. Select its largest child", {835.f, 211.f},
                              13, Palette::TEXT, false);
            renderer.drawText("3. Remove every BAD marker", {835.f, 234.f},
                              13, Palette::TEXT, false);
        } else if (isRepairLevel()) {
            renderer.drawText("Restore the disturbed heap.", {835.f, 158.f},
                              15, Palette::TEXT, false);
            renderer.drawText("1. Select a BAD node", {835.f, 188.f},
                              13, Palette::TEXT, false);
        } else {
            renderer.drawText(level.task == "heap_sort" ? "Complete the full Heap Sort."
                                                         : "Extract maxima into sorted order.",
                              {835.f, 158.f}, 15, Palette::TEXT, false);
            renderer.drawText("Operate on the active heap", {835.f, 188.f},
                              13, Palette::TEXT, false);
            renderer.drawText("Sorted cells lock at the end", {835.f, 211.f},
                              13, Palette::TEXT, false);
            renderer.drawText("Z or Undo restores a snapshot", {835.f, 234.f},
                              13, Palette::TEXT, false);
            renderer.drawText("2. Choose Up or Down", {835.f, 211.f},
                              13, Palette::TEXT, false);
            renderer.drawText("3. Prefer the shorter repair", {835.f, 234.f},
                              13, Palette::TEXT, false);
        }

        alarmMeter.draw(window, renderer, currentScore(), {835.f, 278.f}, 220.f);
        renderer.drawText("STEPS: " + to_string(currentScore().steps()),
                          {835.f, 318.f}, 14, Palette::TEXT, false);
        renderer.drawText("PENALTIES: " + to_string(currentScore().penalties()),
                          {835.f, 343.f}, 14, Palette::TEXT, false);
        renderer.drawText(message, {835.f, 390.f}, 12,
                          currentFailed() ? Palette::VIOLATION : Palette::WARNING,
                          false);
    } else {
        renderer.drawText("ACT 4 LOAD ERROR", {835.f, 80.f},
                          20, Palette::VIOLATION, false);
        renderer.drawText(message, {835.f, 125.f}, 13, Palette::TEXT, false);
    }

    if (levelLoaded && isRepairLevel() && !currentComplete()) {
        drawButton(window, renderer, upButton(), "Heapify Up");
        drawButton(window, renderer, downButton(), "Heapify Down");
    }
    if (levelLoaded && isOperationLevel() && !currentComplete()) {
        drawButton(window, renderer, undoButton(), "Undo");
        drawButton(window, renderer, actionButton(),
                   level.task == "heap_sort" ? "Run Heap Sort" : "Extract Max");
    }
    drawButton(window, renderer, resetButton(), "Reset Level");
    if (levelLoaded && currentComplete()) {
        drawButton(window, renderer, continueButton(), "View Result");
    }
    drawButton(window, renderer, menuButton(), "Main Menu");
    renderer.drawText("R reset | Z undo | P replay | Space pause | arrows step",
                      {550.f, 692.f}, 13, Palette::MUTED, true);
}

}
