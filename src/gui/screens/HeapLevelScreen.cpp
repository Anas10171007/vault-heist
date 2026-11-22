#include "gui/screens/HeapLevelScreen.h"
#include "game/LevelDataBuilder.h"
#include "game/LevelManager.h"
#include "gui/core/Renderer.h"
#include "gui/core/ScreenId.h"

namespace vh {

HeapLevelScreen::HeapLevelScreen(Theme& sharedTheme)
    : theme(sharedTheme), levelLoaded(false), replayBaselineActive(0),
      firstSelection(-1), waitingForAnimation(false), extractDragging(false),
      reviewMode(false) {
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

bool HeapLevelScreen::canReviewReplay() const {
    return levelLoaded && levelTimeline.size() > 0;
}

bool HeapLevelScreen::isReviewMode() const { return reviewMode; }

bool HeapLevelScreen::startReviewReplay() {
    if (!canReviewReplay()) return false;
    reviewMode = true;
    waitingForAnimation = false;
    extractDragging = false;
    firstSelection = -1;
    heapView.setSelectedIndex(-1);
    heapView.load(replayBaseline, replayBaselineActive);
    animation.load(levelTimeline);
    animation.restart();
    animation.play();
    message = "Reviewing your run. Scrub the timeline or press Replay.";
    return true;
}

bool HeapLevelScreen::canSoloExtract() const {
    return isOperationLevel() && !operationLogic.needsBuild() &&
           !operationLogic.needsRestore() && !operationLogic.needsExtractConfirm() &&
           operationLogic.activeSize() == 1;
}

void HeapLevelScreen::beginSoloExtract() {
    if (!canSoloExtract()) return;
    bool accepted = false;
    EventLog events = operationLogic.chooseExtract(0, 0, accepted);
    firstSelection = -1;
    heapView.setSelectedIndex(-1);
    if (!accepted) {
        message = operationLogic.isFailed() ? "Alarm full."
                                            : "Cannot extract the last node.";
        return;
    }
    playOperationEvents(events);
    if (operationLogic.needsExtractConfirm()) {
        message = "Only one node left — drag it into SORTED, or click Extract.";
        heapView.setSelectedIndex(operationLogic.lastActiveIndex());
    }
}

void HeapLevelScreen::resetLevel() {
    firstSelection = -1;
    waitingForAnimation = false;
    extractDragging = false;
    reviewMode = false;
    EventLog empty;
    animation.load(empty);
    if (!levelLoaded) return;

    // Seeded generate (or explicit JSON values) — same seed ⇒ same array every run.
    DynArray<int> values;
    LevelDataBuilder::buildValues(level, values);

    if (isBuildLevel()) {
        buildLogic.load(values);
        message = "Select a smaller parent, then its largest child.";
    } else if (isRepairLevel()) {
        repairLogic.load(values);
        message = "Select a BAD node, then Heapify Up or Heapify Down.";
    } else {
        operationLogic.load(values, level.task, level.rules.count);
        if (level.task == "heap_race") {
            message = "Race: tighter alarm. Extract " + to_string(level.rules.count) +
                      " maxima before it fills.";
        } else if (level.task == "heap_sort") {
            message = "Build Max Heap first, then extract into SORTED.";
        } else {
            message = "Select root + last leaf to swap, then drag max into SORTED.";
        }
    }
    resetReplayTimeline();
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

void HeapLevelScreen::resetReplayTimeline() {
    levelTimeline.clear();
    replayBaseline.clear();
    copyCurrentValues(replayBaseline);
    replayBaselineActive =
        isOperationLevel() ? operationLogic.activeSize() : replayBaseline.size();
    heapView.load(replayBaseline, replayBaselineActive);
    EventLog empty;
    animation.load(empty);
}

void HeapLevelScreen::playOperationEvents(const EventLog& events) {
    int prior = levelTimeline.size();
    levelTimeline.extend(events);
    heapView.load(replayBaseline, replayBaselineActive);
    animation.load(levelTimeline);
    animation.seekTo(prior);
    animation.play();
    waitingForAnimation = true;
}

void HeapLevelScreen::confirmPendingExtract() {
    if (!isOperationLevel() || !operationLogic.needsExtractConfirm()) return;

    bool accepted = false;
    EventLog events = operationLogic.confirmExtract(accepted);
    extractDragging = false;
    firstSelection = -1;
    heapView.setSelectedIndex(-1);

    if (!accepted) {
        message = operationLogic.isFailed() ? "Alarm full. Reset to try again."
                                            : "Drag the last cell into SORTED to seal.";
        return;
    }

    playOperationEvents(events);
    if (operationLogic.needsRestore()) {
        message = "Sealed into SORTED. Swap BAD pairs to restore the heap.";
    } else {
        message = "Final maximum sealed into SORTED.";
    }
}

void HeapLevelScreen::runSelectedHeapify(bool siftUp) {
    if (!isRepairLevel() || firstSelection < 0) {
        message = "Select a BAD node first.";
        return;
    }
    bool accepted = false;
    EventLog events = repairLogic.chooseHeapify(firstSelection, siftUp, accepted);
    int selected = firstSelection;
    firstSelection = -1;
    heapView.setSelectedIndex(-1);
    if (!accepted) {
        message = repairLogic.isFailed() ? "Alarm full."
                                         : (siftUp ? "Cannot Heapify Up here."
                                                   : "Cannot Heapify Down here.");
        firstSelection = selected;
        heapView.setSelectedIndex(selected);
        return;
    }
    playOperationEvents(events);
    message = repairLogic.isComplete() ? "Locks repaired. Max Heap restored."
                                       : "Heapify applied. Continue if BAD remains.";
}

void HeapLevelScreen::runBuildAction() {
    if (!isOperationLevel()) return;
    bool accepted = false;
    EventLog events = operationLogic.runBuildMaxHeap(accepted);
    if (!accepted) {
        message = operationLogic.isFailed() ? "Alarm full." : "Build not needed right now.";
        return;
    }
    playOperationEvents(events);
    message = "Max Heap built. Swap root↔last, then drag into SORTED.";
}

void HeapLevelScreen::runHeapifyRootAction() {
    if (!isOperationLevel()) return;
    bool accepted = false;
    EventLog events = operationLogic.runHeapifyDownRoot(accepted);
    if (!accepted) {
        message = operationLogic.isFailed() ? "Alarm full." : "Heapify only after extract.";
        return;
    }
    playOperationEvents(events);
    message = operationLogic.needsRestore() ? "Still broken. Keep repairing."
                                            : "Heap restored after Heapify.";
}

void HeapLevelScreen::selectIndex(int index) {
    if (isRepairLevel()) {
        if (firstSelection == index) {
            firstSelection = -1;
            heapView.setSelectedIndex(-1);
            message = "Selection cleared.";
            return;
        }
        firstSelection = index;
        heapView.setSelectedIndex(index);
        message = "Now click Heapify Up or Heapify Down.";
        return;
    }

    if (isOperationLevel()) {
        if (operationLogic.needsBuild()) {
            message = "Click Build Max Heap first.";
            return;
        }
        if (operationLogic.needsExtractConfirm()) {
            message = "Drag the last cell into SORTED, or click Extract.";
            return;
        }

        // One node left: root is already the last leaf — no second click / swap.
        if (canSoloExtract() && index == 0) {
            beginSoloExtract();
            return;
        }

        if (firstSelection < 0) {
            firstSelection = index;
            heapView.setSelectedIndex(index);
            if (operationLogic.needsRestore()) {
                message = "Now select the connected larger child.";
            } else {
                message = "Now select the last active leaf to swap with the root.";
            }
            return;
        }

        if (index == firstSelection) {
            firstSelection = -1;
            heapView.setSelectedIndex(-1);
            message = "Selection cleared.";
            return;
        }

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
                message = "Alarm full.";
            } else if (restoring) {
                message = "Invalid parent-child swap.";
            } else {
                message = "Swap needs the root and the last active leaf.";
            }
            return;
        }

        playOperationEvents(events);
        if (restoring) {
            message = operationLogic.needsRestore()
                          ? "Valid swap. Keep repairing BAD pairs."
                          : "Heap restored. Swap root↔last again.";
        } else if (operationLogic.needsExtractConfirm()) {
            message = "Max is at the end. Drag it into SORTED (or click Extract).";
            heapView.setSelectedIndex(operationLogic.lastActiveIndex());
        }
        return;
    }

    // Build: two-click parent-child swaps.
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

    bool accepted = false;
    EventLog events = buildLogic.chooseSwap(firstSelection, index, accepted);
    firstSelection = -1;
    heapView.setSelectedIndex(-1);

    if (!accepted) {
        message = buildLogic.isFailed() ? "Alarm full." : "Invalid parent-child swap.";
        return;
    }

    playOperationEvents(events);
    message = buildLogic.isComplete() ? "Vault activated. Max Heap complete." : "Valid swap.";
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

    firstSelection = -1;
    extractDragging = false;
    waitingForAnimation = false;
    resetReplayTimeline();
    if (isOperationLevel() && operationLogic.needsExtractConfirm()) {
        message = "Undone to pending extract. Drag last cell into SORTED.";
        heapView.setSelectedIndex(operationLogic.lastActiveIndex());
    } else {
        message = "Last operation undone.";
    }
}

void HeapLevelScreen::showHint() {
    if (isBuildLevel()) buildLogic.applyHint();
    else if (isRepairLevel()) repairLogic.applyHint();
    else operationLogic.applyHint();

    if (isBuildLevel()) {
        message = "Hint: pick a BAD parent, then its largest child.";
    } else if (isRepairLevel()) {
        message = "Hint: select a BAD node, then Heapify Up or Down.";
    } else if (operationLogic.needsBuild()) {
        message = "Hint: click Build Max Heap before extracting.";
    } else if (canSoloExtract()) {
        message = "Hint: last node needs no swap — click Extract or the root.";
    } else if (operationLogic.needsExtractConfirm()) {
        message = "Hint: drag the last array cell into SORTED, or click Extract.";
    } else if (operationLogic.needsRestore()) {
        message = "Hint: swap BAD pairs, or Heapify the root.";
    } else {
        int last = operationLogic.lastActiveIndex();
        message = "Hint: select root (0), then last leaf (" + to_string(last) +
                  "), then drag that max into SORTED.";
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
    if (isRepairLevel()) return "Repair with Heapify Up / Heapify Down.";
    if (level.task == "heap_race") {
        return "Race: extract " + to_string(level.rules.count) + " maxima (tight alarm).";
    }
    if (level.task == "heap_sort") return "Build, then extract into SORTED order.";
    return "Select root + last leaf, then restore swaps.";
}

int HeapLevelScreen::handleEvent(const sf::Event& event, sf::RenderWindow& window) {
    const sf::Event::KeyPressed* key = event.getIf<sf::Event::KeyPressed>();
    if (key != nullptr) {
        if (key->code == sf::Keyboard::Key::Escape) {
            return reviewMode ? ScreenId::RESULT : ScreenId::CAMPAIGN_MAP;
        }
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
        if (release->button == sf::Mouse::Button::Left) {
            scrubber.release();
            if (extractDragging) {
                sf::Vector2f drop = window.mapPixelToCoords(release->position);
                bool overSorted = heapView.hitsSortedDrop(drop, {48.f, 470.f}, 700.f);
                extractDragging = false;
                if (overSorted) {
                    confirmPendingExtract();
                    return ScreenId::NONE;
                }
                message = "Drop the last cell on the SORTED slot (or click Extract).";
            }
        }
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
    if (LevelChrome::exitButton().contains(point)) {
        return reviewMode ? ScreenId::RESULT : ScreenId::CAMPAIGN_MAP;
    }

    bool replayAtEnd = animation.completedCount() == animation.eventCount();
    bool canAct = levelLoaded && !reviewMode && !currentComplete() && !currentFailed() &&
                  !waitingForAnimation && replayAtEnd;
    if (!canAct) return ScreenId::NONE;

    if (isRepairLevel()) {
        if (LevelChrome::opButtonA().contains(point)) {
            runSelectedHeapify(true);
            return ScreenId::NONE;
        }
        if (LevelChrome::opButtonB().contains(point)) {
            runSelectedHeapify(false);
            return ScreenId::NONE;
        }
    }
    if (isOperationLevel() && LevelChrome::opButtonWide().contains(point)) {
        if (operationLogic.needsBuild()) {
            runBuildAction();
            return ScreenId::NONE;
        }
        if (operationLogic.needsExtractConfirm()) {
            confirmPendingExtract();
            return ScreenId::NONE;
        }
        if (canSoloExtract()) {
            beginSoloExtract();
            return ScreenId::NONE;
        }
        if (operationLogic.needsRestore()) {
            runHeapifyRootAction();
            return ScreenId::NONE;
        }
    }

    int index = heapView.hitTest(point, {48.f, 70.f}, 700.f,
                                 {48.f, 470.f}, 700.f);
    if (isOperationLevel() && operationLogic.needsExtractConfirm() &&
        index == operationLogic.lastActiveIndex()) {
        extractDragging = true;
        heapView.setSelectedIndex(index);
        message = "Drag into the SORTED slot, then release.";
        return ScreenId::NONE;
    }
    if (index >= 0) selectIndex(index);
    return ScreenId::NONE;
}

int HeapLevelScreen::update(float seconds) {
    animation.update(seconds);
    if (reviewMode) return ScreenId::NONE;

    if (currentFailed() && !waitingForAnimation) {
        message = "Alarm full. Mission failed.";
        return ScreenId::RESULT;
    }
    if (waitingForAnimation && !animation.isPlaying() &&
        animation.completedCount() == animation.eventCount()) {
        waitingForAnimation = false;
        if (currentFailed()) {
            message = "Alarm full. Mission failed.";
            return ScreenId::RESULT;
        }
        if (currentComplete()) {
            if (isBuildLevel()) message = "Vault activated. Max Heap complete.";
            else if (isRepairLevel()) message = "Security locks repaired. Max Heap restored.";
            else if (level.task == "heap_race") message = "Race complete. Alarm beaten.";
            else message = "Vault cleared. Sorted sequence complete.";
            return ScreenId::RESULT;
        } else if (isRepairLevel()) {
            message = "Select a BAD node, then Heapify Up or Down.";
        } else if (isBuildLevel()) {
            message = "Valid swap. Repair the next violation.";
        } else if (operationLogic.needsBuild()) {
            message = "Click Build Max Heap to start the sort.";
        } else if (operationLogic.needsExtractConfirm()) {
            message = "Max is at the end. Drag it into SORTED (or click Extract).";
            heapView.setSelectedIndex(operationLogic.lastActiveIndex());
        } else if (canSoloExtract()) {
            message = "Last node — click it or Extract (no swap needed).";
        } else if (operationLogic.needsRestore()) {
            message = "Swap BAD pairs or click Heapify.";
        } else {
            message = "Select root + last leaf to swap, then drag max into SORTED.";
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
        heapView.draw(renderer, {48.f, 70.f}, 700.f, {48.f, 470.f}, 700.f);
        chrome.drawHud(window, renderer, theme, currentScore(), level.id, level.title,
                       objectiveText(), message, currentFailed());
        int highlight = animation.completedCount() - 1;
        chrome.drawEventLog(renderer, animation.log(), highlight);
    } else {
        renderer.drawText("ACT 4 LOAD ERROR", {808.f, 80.f},
                          20, Palette::VIOLATION, false);
        renderer.drawText(message, {808.f, 125.f}, 13, Palette::TEXT, false);
    }

    // Only show the ops that matter for the current state — keeps the rail calm.
    if (levelLoaded && !reviewMode && !currentComplete() && !currentFailed()) {
        if (isRepairLevel()) {
            chrome.drawButton(window, theme, renderer, LevelChrome::opButtonA(), "Up");
            chrome.drawButton(window, theme, renderer, LevelChrome::opButtonB(), "Down");
        } else if (isOperationLevel()) {
            if (operationLogic.needsBuild()) {
                chrome.drawButton(window, theme, renderer, LevelChrome::opButtonWide(),
                                  "Build Max Heap", ButtonKind::PRIMARY);
            } else if (operationLogic.needsExtractConfirm() || canSoloExtract()) {
                chrome.drawButton(window, theme, renderer, LevelChrome::opButtonWide(),
                                  "Extract to Sorted", ButtonKind::PRIMARY);
                if (operationLogic.needsExtractConfirm()) {
                    sf::Vector2f drop = heapView.sortedDropCenter({48.f, 470.f}, 700.f);
                    renderer.drawText("DROP", {drop.x, drop.y - 28.f}, 11, Palette::BRASS, true);
                    renderer.drawArrayCell(drop, operationLogic.lastActiveIndex(),
                                           heapView.valueAt(operationLogic.lastActiveIndex()),
                                           extractDragging ? CellRegion::SELECTED
                                                           : CellRegion::SORTED);
                }
            } else if (operationLogic.needsRestore()) {
                chrome.drawButton(window, theme, renderer, LevelChrome::opButtonWide(),
                                  "Heapify Root");
            }
        }
    }
    if (levelLoaded && !reviewMode && !currentComplete()) {
        chrome.drawButton(window, theme, renderer, LevelChrome::undoButton(), "Undo");
    }
    if (!reviewMode) {
        chrome.drawButton(window, theme, renderer, LevelChrome::hintButton(), "Hint");
    }
    chrome.drawButton(window, theme, renderer, LevelChrome::replayButton(), "Replay");
    if (!reviewMode) {
        chrome.drawButton(window, theme, renderer, LevelChrome::resetButton(), "Reset",
                          ButtonKind::DANGER);
    }
    chrome.drawButton(window, theme, renderer, LevelChrome::slowerButton(), "Slow",
                      ButtonKind::GHOST);
    chrome.drawButton(window, theme, renderer, LevelChrome::fasterButton(), "Fast",
                      ButtonKind::GHOST);
    chrome.drawButton(window, theme, renderer, LevelChrome::exitButton(),
                      reviewMode ? "Results" : "Campaign", ButtonKind::SECONDARY);
    scrubber.draw(window, renderer, theme, animation);
}

}
