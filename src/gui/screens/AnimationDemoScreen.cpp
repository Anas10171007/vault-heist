#include "gui/screens/AnimationDemoScreen.h"

namespace vh {

AnimationDemoScreen::AnimationDemoScreen(Theme& theme)
    : StubScreen(theme, "Level Play", "Five-event animation demonstration"), demoTheme(theme) {
    addButton("Finish Demo Level", ScreenId::RESULT);
    addButton("Main Menu", ScreenId::MENU);
    animation.setListener(this);
    EventLog demo;
    demo.append(Event(EventType::VISIT, 0));
    demo.append(Event(EventType::COMPARE, 1));
    demo.append(Event(EventType::INSERT, 2));
    demo.append(Event(EventType::UPDATE_SIZE, 3));
    demo.append(Event(EventType::DONE, 4));
    animation.load(demo);
}

void AnimationDemoScreen::reset() {
    for (int i = 0; i < 5; i++) progress[i] = 0;
}

void AnimationDemoScreen::onEvent(const Event& event, float t) {
    // Assign from progress, rather than adding: callbacks may repeat an event.
    if (event.idA >= 0 && event.idA < 5) progress[event.idA] = t;
}

int AnimationDemoScreen::handleEvent(const sf::Event& event, sf::RenderWindow& window) {
    const sf::Event::KeyPressed* key = event.getIf<sf::Event::KeyPressed>();
    if (key != nullptr) {
        if (key->code == sf::Keyboard::Key::Space) {
            if (animation.isPlaying()) animation.pause();
            else animation.play();
        } else if (key->code == sf::Keyboard::Key::Right) animation.stepForward();
        else if (key->code == sf::Keyboard::Key::Left) animation.stepBackward();
        else if (key->code == sf::Keyboard::Key::Up) animation.setSpeed(animation.getSpeed() * 2);
        else if (key->code == sf::Keyboard::Key::Down) animation.setSpeed(animation.getSpeed() / 2);
        else if (key->code == sf::Keyboard::Key::R) animation.restart();
        else if (key->code == sf::Keyboard::Key::I) animation.finishInstantly();
    }
    return StubScreen::handleEvent(event, window);
}

int AnimationDemoScreen::update(float seconds) {
    animation.update(seconds);
    return ScreenId::NONE;
}

void AnimationDemoScreen::draw(sf::RenderWindow& window) {
    StubScreen::draw(window);
    const char* names[5] = {"VISIT", "COMPARE", "INSERT", "SIZE", "DONE"};
    for (int i = 0; i < 5; i++) {
        float x = 270.f + i * 112.f;
        sf::RectangleShape track({98.f, 20.f});
        track.setPosition({x, 440.f});
        track.setFillColor(Palette::MUTED);
        window.draw(track);
        sf::RectangleShape fill({98.f * progress[i], 20.f});
        fill.setPosition({x, 440.f});
        fill.setFillColor(Palette::TARGET);
        window.draw(fill);
        sf::Text label = demoTheme.makeLabel(names[i], 16, Palette::TEXT);
        label.setPosition({x, 408.f});
        window.draw(label);
    }
    string status = animation.isPlaying() ? "Playing" : "Paused";
    if (animation.completedCount() == animation.eventCount()) status = "Complete";
    status += " | Events: " + to_string(animation.completedCount()) + "/5 | Speed: ";
    double speed = animation.getSpeed();
    if (speed == 0.25) status += "0.25x";
    else if (speed == 0.5) status += "0.5x";
    else status += to_string(static_cast<int>(speed)) + "x";
    sf::Text state = demoTheme.makeLabel(status, 20, Palette::TEXT);
    state.setPosition({270.f, 480.f});
    window.draw(state);
    sf::Text controls = demoTheme.makeLabel(
        "Space: play/pause | Left/Right: step\nUp/Down: speed | R: restart | I: instant", 18, Palette::TEXT);
    controls.setPosition({270.f, 525.f});
    window.draw(controls);
}

}
