#include "gui/screens/HeapLevelScreen.h"
#include "game/LevelManager.h"
#include "gui/core/Renderer.h"
#include "gui/core/ScreenId.h"

namespace vh {

HeapLevelScreen::HeapLevelScreen(Theme& sharedTheme)
    : theme(sharedTheme), levelLoaded(false), firstSelection(-1),
      waitingForAnimation(false) {
    animation.setListener(&heapView);
    levelLoaded = loadLevel();
    resetLevel();
}

bool HeapLevelScreen::loadLevel() {
    const char* folders[4] = {"data/levels", "../data/levels",
                              "../../data/levels", "../../../data/levels"};
    for (int index = 0; index < 4; index++) {
        LevelManager manager;
        if (!manager.loadAll(folders[index])) continue;
        const LevelDef* found = manager.load("4.1");
        if (found != nullptr) {
            level = *found;
            return true;
        }
    }
    message = "Could not load data/levels/act4.json";
    return false;
}

void HeapLevelScreen::resetLevel() {
    firstSelection = -1;
    waitingForAnimation = false;
    EventLog empty;
    animation.load(empty);
    if (!levelLoaded) return;

    logic.load(level.data.values);
    heapView.load(level.data.values, level.data.values.size());
    message = "Select a smaller parent, then its largest child.";
}

void HeapLevelScreen::onShow() {
    resetLevel();
}

void HeapLevelScreen::selectIndex(int index) {
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
    logic.copyValues(before);
    bool accepted = false;
    EventLog events = logic.chooseSwap(firstSelection, index, accepted);
    firstSelection = -1;
    heapView.setSelectedIndex(-1);

    if (!accepted) {
        message = logic.isFailed() ? "Alarm full. Reset the level to try again."
                                   : "Invalid swap: choose a parent and its largest child.";
        return;
    }

    heapView.load(before, before.size());
    animation.load(events);
    animation.play();
    waitingForAnimation = true;
    message = logic.isComplete() ? "Final valid swap..." : "Valid swap.";
}

int HeapLevelScreen::handleEvent(const sf::Event& event, sf::RenderWindow& window) {
    const sf::Event::KeyPressed* key = event.getIf<sf::Event::KeyPressed>();
    if (key != nullptr) {
        if (key->code == sf::Keyboard::Key::R) resetLevel();
        else if (key->code == sf::Keyboard::Key::Space) {
            if (animation.isPlaying()) animation.pause();
            else animation.play();
        } else if (key->code == sf::Keyboard::Key::Right) animation.stepForward();
        else if (key->code == sf::Keyboard::Key::Left) animation.stepBackward();
        else if (key->code == sf::Keyboard::Key::Enter && logic.isComplete()) {
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
    if (menuButton().contains(point)) return ScreenId::MENU;
    if (logic.isComplete() && continueButton().contains(point)) return ScreenId::RESULT;
    bool replayAtEnd = animation.completedCount() == animation.eventCount();
    if (!levelLoaded || logic.isComplete() || logic.isFailed() ||
        waitingForAnimation || !replayAtEnd) {
        return ScreenId::NONE;
    }

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
        message = logic.isComplete() ? "Vault activated. The array is now a Max Heap."
                                     : "Valid swap. Repair the next violation.";
    }
}

sf::FloatRect HeapLevelScreen::resetButton() const {
    return {{840.f, 490.f}, {210.f, 42.f}};
}

sf::FloatRect HeapLevelScreen::continueButton() const {
    return {{840.f, 542.f}, {210.f, 42.f}};
}

sf::FloatRect HeapLevelScreen::menuButton() const {
    return {{840.f, 594.f}, {210.f, 42.f}};
}

void HeapLevelScreen::drawButton(sf::RenderWindow& window, Renderer& renderer,
                                 sf::FloatRect bounds, const string& label) const {
    sf::RectangleShape button = theme.makeButton(bounds.position, bounds.size);
    window.draw(button);
    renderer.drawText(label, bounds.position + bounds.size / 2.f,
                      17, Palette::TEXT, true);
}

void HeapLevelScreen::draw(sf::RenderWindow& window) {
    Renderer renderer(window, theme.getFont());
    sf::RectangleShape panel = theme.makePanel({815.f, 45.f}, {260.f, 620.f});
    window.draw(panel);

    if (levelLoaded) {
        heapView.draw(renderer, {40.f, 105.f}, 750.f, {40.f, 520.f}, 750.f);
        renderer.drawText(level.id + "  " + level.title, {835.f, 70.f},
                          22, Palette::TARGET, false);
        renderer.drawText("ACTIVATE THE VAULT", {835.f, 108.f},
                          13, Palette::MUTED, false);
        renderer.drawText("Build a Max Heap manually.", {835.f, 145.f},
                          16, Palette::TEXT, false);
        renderer.drawText("1. Select a smaller parent", {835.f, 177.f},
                          14, Palette::TEXT, false);
        renderer.drawText("2. Select its largest child", {835.f, 201.f},
                          14, Palette::TEXT, false);
        renderer.drawText("3. Repeat until all BAD", {835.f, 225.f},
                          14, Palette::TEXT, false);
        renderer.drawText("   markers disappear", {835.f, 249.f},
                          14, Palette::TEXT, false);

        alarmMeter.draw(window, renderer, logic.score(), {835.f, 292.f}, 220.f);
        renderer.drawText("STEPS: " + to_string(logic.score().steps()),
                          {835.f, 332.f}, 15, Palette::TEXT, false);
        renderer.drawText("PENALTIES: " + to_string(logic.score().penalties()),
                          {835.f, 358.f}, 15, Palette::TEXT, false);
        renderer.drawText(message, {835.f, 405.f}, 13,
                          logic.isFailed() ? Palette::VIOLATION : Palette::WARNING,
                          false);
    } else {
        renderer.drawText("ACT 4 LOAD ERROR", {835.f, 80.f},
                          20, Palette::VIOLATION, false);
        renderer.drawText(message, {835.f, 125.f}, 13, Palette::TEXT, false);
    }

    drawButton(window, renderer, resetButton(), "Reset Level");
    if (levelLoaded && logic.isComplete()) {
        drawButton(window, renderer, continueButton(), "View Result");
    }
    drawButton(window, renderer, menuButton(), "Main Menu");
    renderer.drawText("R reset | Space pause | arrows step",
                      {550.f, 692.f}, 13, Palette::MUTED, true);
}

}
