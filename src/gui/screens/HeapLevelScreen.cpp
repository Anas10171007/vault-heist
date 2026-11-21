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
        message = "Select a smaller parent, then its largest child.";
    } else {
        operationLogic.load(level.data.values, level.task, level.rules.count);
        if (level.task == "heap_race") {
            message = "Select root + last leaf to extract. Goal: " +
                      to_string(level.rules.count) + " maxima.";
        } else if (level.task == "heap_sort") {
            message = "Select root + last leaf, then restore BAD pairs.";
        } else {
            message = "Select root and last active leaf to extract.";
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
    if (isOperationLevel()) {
        if (firstSelection < 0) {
            firstSelection = index;
            heapView.setSelectedIndex(index);
            if (operationLogic.needsRestore()) {
                message = "Now select the connected larger child.";
            } else if (operationLogic.activeSize() == 1) {
                message = "Only the root remains. Click it again to extract.";
            } else {
                message = "Now select the last active leaf (end of the heap).";
            }
            return;
        }

        if (index == firstSelection && operationLogic.activeSize() != 1) {
            firstSelection = -1;
            heapView.setSelectedIndex(-1);
            message = "Selection cleared.";
            return;
        }

        DynArray<int> before;
        operationLogic.copyValues(before);
        int beforeActive = operationLogic.activeSize();
        bool restoring = operationLogic.needsRestore();
        bool accepted = false;
        EventLog events;
        if (restoring) {
            events = operationLogic.chooseSwap(firstSelection, index, accepted);
        } else {
            events = operationLogic.chooseExtract(firstSelection, index, accepted);
        }
        firstSelection = -1;
        heapView.setSelectedIndex(-1);

        if (!accepted) {
            if (operationLogic.isFailed()) {
                message = "Alarm full. Reset to try again.";
            } else if (restoring) {
                message = "Invalid parent-child swap.";
            } else {
                message = "Extract needs the root and the last active leaf.";
            }
            return;
        }

        heapView.load(before, beforeActive);
        animation.load(events);
        animation.play();
        waitingForAnimation = true;
        if (restoring) {
            message = operationLogic.needsRestore()
                          ? "Valid swap. Keep repairing BAD pairs."
                          : "Heap restored. Select root + last leaf again.";
        } else if (operationLogic.needsRestore()) {
            message = "Extracted. Swap BAD parent-child pairs to restore.";
        } else {
            message = "Final maximum extracted.";
        }
        return;
    }

    // Build and repair: two-click parent-child swaps.
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
    bool accepted = false;
    EventLog events;
    if (isRepairLevel()) {
        repairLogic.copyValues(before);
        events = repairLogic.chooseSwap(firstSelection, index, accepted);
    } else {
        buildLogic.copyValues(before);
        events = buildLogic.chooseSwap(firstSelection, index, accepted);
    }
    firstSelection = -1;
    heapView.setSelectedIndex(-1);

    if (!accepted) {
        bool failed = isRepairLevel() ? repairLogic.isFailed() : buildLogic.isFailed();
        message = failed ? "Alarm full. Reset to try again." : "Invalid parent-child swap.";
        return;
    }

    heapView.load(before, before.size());
    animation.load(events);
    animation.play();
    waitingForAnimation = true;
    bool done = isRepairLevel() ? repairLogic.isComplete() : buildLogic.isComplete();
    message = done ? "Final valid swap..." : "Valid swap.";
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
    if (isBuildLevel() || isRepairLevel()) {
        message = "Hint: pick a BAD parent, then its largest child.";
    } else if (operationLogic.needsRestore()) {
        message = "Hint: swap each BAD parent with its larger child.";
    } else {
        int last = operationLogic.lastActiveIndex();
        message = "Hint: select root (0), then last leaf (" + to_string(last) + ").";
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
    if (isRepairLevel()) return "Repair the disturbed heap by swapping BAD pairs.";
    if (level.task == "heap_race") {
        return "Extract " + to_string(level.rules.count) + " maxima (root+last leaf).";
    }
    if (level.task == "heap_sort") return "Heap Sort: root+last leaf, then restore.";
    return "Select root + last leaf, then restore swaps.";
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
    }

    if (const auto* release = event.getIf<sf::Event::MouseButtonReleased>()) {
        if (release->button == sf::Mouse::Button::Left) scrubber.release();
    }
    if (scrubber.isDragging()) {
        scrubber.handleDrag(window, animation);
        waitingForAnimation = false;
    }

    const sf::Event::MouseButtonPressed* click = event.getIf<sf::Event::MouseButtonPressed>();
    if (click == nullptr || click->button != sf::Mouse::Button::Left) {
        return ScreenId::NONE;
    }

    sf::Vector2f point = window.mapPixelToCoords(click->position);
    if (scrubber.handleClick(window, point, animation)) {
        waitingForAnimation = false;
        return ScreenId::NONE;
    }
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

    bool replayAtEnd = animation.completedCount() == animation.eventCount();
    bool canAct = levelLoaded && !currentComplete() && !currentFailed() &&
                  !waitingForAnimation && replayAtEnd;
    if (!canAct) return ScreenId::NONE;

    int index = heapView.hitTest(point, {48.f, 70.f}, 700.f,
                                 {48.f, 520.f}, 700.f);
    if (index >= 0) selectIndex(index);
    return ScreenId::NONE;
}

int HeapLevelScreen::update(float seconds) {
    animation.update(seconds);
    if (waitingForAnimation && !animation.isPlaying() &&
        animation.completedCount() == animation.eventCount()) {
        waitingForAnimation = false;
        if (currentComplete()) {
            if (isBuildLevel()) message = "Vault activated. Max Heap complete.";
            else if (isRepairLevel()) message = "Security locks repaired. Max Heap restored.";
            else if (level.task == "heap_race") message = "Race complete. Alarm beaten.";
            else message = "Vault cleared. Sorted sequence complete.";
            return ScreenId::RESULT;
        } else if (isBuildLevel() || isRepairLevel()) {
            message = "Valid swap. Repair the next violation.";
        } else if (operationLogic.needsRestore()) {
            message = "Swap BAD parent-child pairs to restore the Max Heap.";
        } else {
            message = "Select root and last active leaf to extract.";
        }
    }
    return ScreenId::NONE;
}

void HeapLevelScreen::draw(sf::RenderWindow& window) {
    theme.drawBackdrop(window);
    Renderer renderer(window, theme.getFont());
    chrome.drawCanvasPanel(window, theme);
    chrome.drawSidePanel(window, theme);

    if (levelLoaded) {
        heapView.draw(renderer, {48.f, 70.f}, 700.f, {48.f, 520.f}, 700.f);
        chrome.drawHud(window, renderer, theme, currentScore(), level.id, level.title,
                       objectiveText(), message, currentFailed());
        int highlight = animation.completedCount() - 1;
        chrome.drawEventLog(renderer, animation.log(), highlight);
    } else {
        renderer.drawText("ACT 4 LOAD ERROR", {804.f, 80.f},
                          20, Palette::VIOLATION, false);
        renderer.drawText(message, {804.f, 125.f}, 13, Palette::TEXT, false);
    }

    if (levelLoaded && isOperationLevel() && !currentComplete()) {
        string guide = operationLogic.needsRestore()
                           ? "Restore: swap BAD pairs"
                           : ("Extract: root + leaf " +
                              to_string(operationLogic.lastActiveIndex()));
        renderer.drawText(guide, {804.f, 330.f}, 12, Palette::VISITED, false);
    }
    if (levelLoaded && !currentComplete()) {
        chrome.drawButton(window, theme, renderer, LevelChrome::undoButton(), "Undo [Z]");
    }
    chrome.drawButton(window, theme, renderer, LevelChrome::hintButton(), "Hint [H]");
    chrome.drawButton(window, theme, renderer, LevelChrome::replayButton(), "Replay");
    chrome.drawButton(window, theme, renderer, LevelChrome::resetButton(), "Reset [R]",
                      ButtonKind::DANGER);
    chrome.drawButton(window, theme, renderer, LevelChrome::slowerButton(), "Slower",
                      ButtonKind::GHOST);
    chrome.drawButton(window, theme, renderer, LevelChrome::fasterButton(), "Faster",
                      ButtonKind::GHOST);
    chrome.drawButton(window, theme, renderer, LevelChrome::exitButton(), "Campaign [Esc]",
                      ButtonKind::SECONDARY);
    scrubber.draw(window, renderer, theme, animation);
    chrome.drawHelp(renderer,
                    "Scrubber · Z undo · H hint · P replay · Space · arrows · Esc");
}

}
