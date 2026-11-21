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
        operationLogic.load(level.data.values, level.task, level.rules.count);
        if (level.task == "heap_race") {
            message = "Select the root, Extract Max, then swap to restore. Goal: " +
                      to_string(level.rules.count) + " maxima.";
        } else if (level.task == "heap_sort") {
            message = "Select the root, Extract Max, then swap BAD pairs until sorted.";
        } else {
            message = "Select the root, then Extract Max. Restore with parent-child swaps.";
        }
    }
    DynArray<int> values;
    copyCurrentValues(values);
    int active = isOperationLevel() ? operationLogic.activeSize() : values.size();
    heapView.load(values, active);
}

void HeapLevelScreen::onShow() {
    resetLevel();
}

bool HeapLevelScreen::isFinalVaultLevel() const {
    return level.task == "heap_sort";
}

string HeapLevelScreen::sortedArrayText() const {
    DynArray<int> values;
    copyCurrentValues(values);
    string text;
    for (int index = 0; index < values.size(); index++) {
        if (index > 0) text += ", ";
        text += to_string(values[index]);
    }
    return text;
}

void HeapLevelScreen::selectIndex(int index) {
    if (isRepairLevel()) {
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

    // Operation levels: select the root to Extract, or two nodes to restore.
    if (isOperationLevel()) {
        if (!operationLogic.needsRestore()) {
            if (index == firstSelection) {
                firstSelection = -1;
                heapView.setSelectedIndex(-1);
                message = "Selection cleared.";
            } else {
                firstSelection = index;
                heapView.setSelectedIndex(index);
                if (index == 0) message = "Root selected. Click Extract Max.";
                else message = "Select the root (top node) before Extract Max.";
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
        operationLogic.copyValues(before);
        int beforeActive = operationLogic.activeSize();
        bool accepted = false;
        EventLog events = operationLogic.chooseSwap(firstSelection, index, accepted);
        firstSelection = -1;
        heapView.setSelectedIndex(-1);

        if (!accepted) {
            message = operationLogic.isFailed() ? "Alarm full. Reset to try again."
                                                : "Invalid parent-child swap.";
            return;
        }

        heapView.load(before, beforeActive);
        animation.load(events);
        animation.play();
        waitingForAnimation = true;
        message = operationLogic.needsRestore()
                      ? "Valid swap. Keep repairing BAD pairs."
                      : "Heap restored. Select the root to extract again.";
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

void HeapLevelScreen::extractOperation() {
    if (firstSelection != 0) {
        message = "Select the root (top node) first, then Extract Max.";
        return;
    }

    DynArray<int> before;
    operationLogic.copyValues(before);
    int beforeActive = operationLogic.activeSize();
    EventLog events = operationLogic.extractMaximum();
    firstSelection = -1;
    heapView.setSelectedIndex(-1);

    if (events.empty()) {
        message = operationLogic.isFailed() ? "Alarm full. Reset to try again."
                                            : "Restore the heap with swaps first.";
        return;
    }

    heapView.load(before, beforeActive);
    animation.load(events);
    animation.play();
    waitingForAnimation = true;
    message = operationLogic.needsRestore()
                  ? "Maximum extracted. Swap BAD parent-child pairs to restore."
                  : "Final maximum extracted.";
}

void HeapLevelScreen::undoOperation() {
    if (waitingForAnimation) {
        message = "Wait for the animation to finish.";
        return;
    }

    bool restoredOk = false;
    if (isBuildLevel()) restoredOk = buildLogic.undo();
    else if (isRepairLevel()) restoredOk = repairLogic.undo();
    else if (isOperationLevel()) restoredOk = operationLogic.undo();

    if (!restoredOk) {
        message = "Nothing to undo.";
        return;
    }

    DynArray<int> restored;
    copyCurrentValues(restored);
    int active = isOperationLevel() ? operationLogic.activeSize() : restored.size();
    heapView.load(restored, active);
    firstSelection = -1;
    heapView.setSelectedIndex(-1);

    EventLog empty;
    animation.load(empty);
    waitingForAnimation = false;
    message = "Last operation undone.";
}

void HeapLevelScreen::showHint() {
    if (isBuildLevel()) {
        message = "Hint: pick a BAD parent, then its largest child.";
    } else if (isRepairLevel()) {
        message = "Hint: select a BAD node, then the shorter heapify direction.";
    } else if (operationLogic.needsRestore()) {
        message = "Hint: swap each BAD parent with its larger child.";
    } else {
        message = "Hint: select the root (index 0), then Extract Max.";
    }
}

void HeapLevelScreen::adjustSpeed(bool faster) {
    double next = animation.getSpeed() * (faster ? 1.25 : 0.8);
    animation.setSpeed(next);
    int hundredths = static_cast<int>(animation.getSpeed() * 100.0 + 0.5);
    message = "Animation speed: " + to_string(hundredths / 100) + "." +
              to_string(hundredths % 100) + "x";
}

string HeapLevelScreen::objectiveText() const {
    if (isBuildLevel()) return "Build a Max Heap by swapping BAD pairs.";
    if (isRepairLevel()) return "Repair the disturbed heap with Up/Down.";
    if (level.task == "heap_race") {
        return "Extract " + to_string(level.rules.count) + " maxima before alarm.";
    }
    if (level.task == "heap_sort") return "Finish Heap Sort: extract and restore.";
    return "Extract max, then restore with swaps.";
}

int HeapLevelScreen::handleEvent(const sf::Event& event, sf::RenderWindow& window) {
    const sf::Event::KeyPressed* key = event.getIf<sf::Event::KeyPressed>();
    if (key != nullptr) {
        if (key->code == sf::Keyboard::Key::Escape) return ScreenId::CAMPAIGN_MAP;
        if (key->code == sf::Keyboard::Key::Num1) loadLevel("4.1");
        else if (key->code == sf::Keyboard::Key::Num2) loadLevel("4.2");
        else if (key->code == sf::Keyboard::Key::Num3) loadLevel("4.3");
        else if (key->code == sf::Keyboard::Key::Num4) loadLevel("4.4");
        else if (key->code == sf::Keyboard::Key::Num5) loadLevel("4.5");
        else if (key->code == sf::Keyboard::Key::R) resetLevel();
        else if (key->code == sf::Keyboard::Key::Z) undoOperation();
        else if (key->code == sf::Keyboard::Key::H) showHint();
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
    if (LevelChrome::resetButton().contains(point)) {
        resetLevel();
        return ScreenId::NONE;
    }
    if (levelLoaded && LevelChrome::undoButton().contains(point)) {
        undoOperation();
        return ScreenId::NONE;
    }
    if (LevelChrome::hintButton().contains(point)) {
        showHint();
        return ScreenId::NONE;
    }
    if (LevelChrome::replayButton().contains(point)) {
        animation.restart();
        animation.play();
        return ScreenId::NONE;
    }
    if (LevelChrome::slowerButton().contains(point)) {
        adjustSpeed(false);
        return ScreenId::NONE;
    }
    if (LevelChrome::fasterButton().contains(point)) {
        adjustSpeed(true);
        return ScreenId::NONE;
    }
    if (LevelChrome::exitButton().contains(point)) return ScreenId::CAMPAIGN_MAP;
    if (levelLoaded && currentComplete() && LevelChrome::continueButton().contains(point)) {
        return ScreenId::RESULT;
    }

    bool replayAtEnd = animation.completedCount() == animation.eventCount();
    bool canAct = levelLoaded && !currentComplete() && !currentFailed() &&
                  !waitingForAnimation && replayAtEnd;
    if (isOperationLevel() && canAct && LevelChrome::opButtonWide().contains(point)) {
        extractOperation();
        return ScreenId::NONE;
    }
    if (isRepairLevel() && canAct && LevelChrome::opButtonA().contains(point)) {
        chooseDirection(HeapDirection::UP);
        return ScreenId::NONE;
    }
    if (isRepairLevel() && canAct && LevelChrome::opButtonB().contains(point)) {
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
            else if (level.task == "heap_race") message = "Race complete. Alarm beaten.";
            else message = "Vault cleared. Sorted sequence complete.";
        } else if (isBuildLevel()) {
            message = "Valid swap. Repair the next violation.";
        } else if (isRepairLevel()) {
            message = "Valid repair. Select the next BAD node.";
        } else if (operationLogic.needsRestore()) {
            message = "Swap BAD parent-child pairs to restore the Max Heap.";
        } else {
            message = "Select the root, then click Extract Max.";
        }
    }
}

void HeapLevelScreen::draw(sf::RenderWindow& window) {
    Renderer renderer(window, theme.getFont());
    chrome.drawSidePanel(window, theme);

    if (levelLoaded) {
        heapView.draw(renderer, {40.f, 105.f}, 750.f, {40.f, 520.f}, 750.f);
        chrome.drawHud(window, renderer, currentScore(), level.id, level.title,
                       objectiveText(), message, currentFailed());
        int highlight = animation.completedCount() - 1;
        chrome.drawEventLog(renderer, animation.log(), highlight);
    } else {
        renderer.drawText("ACT 4 LOAD ERROR", {820.f, 80.f},
                          20, Palette::VIOLATION, false);
        renderer.drawText(message, {820.f, 125.f}, 13, Palette::TEXT, false);
    }

    if (levelLoaded && isRepairLevel() && !currentComplete()) {
        chrome.drawButton(window, theme, renderer, LevelChrome::opButtonA(), "Up");
        chrome.drawButton(window, theme, renderer, LevelChrome::opButtonB(), "Down");
    }
    if (levelLoaded && isOperationLevel() && !currentComplete()) {
        chrome.drawButton(window, theme, renderer, LevelChrome::opButtonWide(), "Extract Max");
    }
    if (levelLoaded && !currentComplete()) {
        chrome.drawButton(window, theme, renderer, LevelChrome::undoButton(), "Undo [Z]");
    }
    chrome.drawButton(window, theme, renderer, LevelChrome::hintButton(), "Hint [H]");
    chrome.drawButton(window, theme, renderer, LevelChrome::replayButton(), "Replay");
    chrome.drawButton(window, theme, renderer, LevelChrome::resetButton(), "Reset [R]");
    chrome.drawButton(window, theme, renderer, LevelChrome::slowerButton(), "Slower");
    chrome.drawButton(window, theme, renderer, LevelChrome::fasterButton(), "Faster");
    chrome.drawButton(window, theme, renderer, LevelChrome::exitButton(), "Campaign [Esc]");
    if (levelLoaded && currentComplete()) {
        chrome.drawButton(window, theme, renderer, LevelChrome::continueButton(), "View Result");
    }
    chrome.drawHelp(renderer,
                    "Z undo | H hint | P replay | Space pause | arrows step | Esc map");
}

}
