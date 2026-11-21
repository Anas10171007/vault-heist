#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include "game/ScoreSystem.h"
#include "gui/core/Renderer.h"
#include "gui/core/Theme.h"
#include "gui/widgets/AlarmMeter.h"
#include "gui/widgets/EventLogPanel.h"
#include "model/EventLog.h"

using namespace std;

namespace vh {

// Shared level frame for Acts 1-4: side panel HUD, event log, toolbar slots.
struct LevelChrome {
    static sf::FloatRect canvasBounds() { return {{40.f, 45.f}, {740.f, 610.f}}; }
    static sf::FloatRect sidePanelBounds() { return {{800.f, 45.f}, {270.f, 610.f}}; }

    static sf::FloatRect undoButton() { return {{820.f, 455.f}, {110.f, 32.f}}; }
    static sf::FloatRect hintButton() { return {{940.f, 455.f}, {110.f, 32.f}}; }
    static sf::FloatRect replayButton() { return {{820.f, 492.f}, {110.f, 32.f}}; }
    static sf::FloatRect resetButton() { return {{940.f, 492.f}, {110.f, 32.f}}; }
    static sf::FloatRect slowerButton() { return {{820.f, 529.f}, {110.f, 32.f}}; }
    static sf::FloatRect fasterButton() { return {{940.f, 529.f}, {110.f, 32.f}}; }
    static sf::FloatRect exitButton() { return {{820.f, 568.f}, {230.f, 34.f}}; }
    static sf::FloatRect continueButton() { return {{820.f, 608.f}, {230.f, 34.f}}; }

    // Extra operation slots (heapify / extract / next-step).
    static sf::FloatRect opButtonA() { return {{820.f, 300.f}, {110.f, 32.f}}; }
    static sf::FloatRect opButtonB() { return {{940.f, 300.f}, {110.f, 32.f}}; }
    static sf::FloatRect opButtonWide() { return {{820.f, 300.f}, {230.f, 32.f}}; }

    void drawSidePanel(sf::RenderWindow& window, Theme& theme) const {
        window.draw(theme.makePanel(sidePanelBounds().position, sidePanelBounds().size));
    }

    void drawCanvasPanel(sf::RenderWindow& window, Theme& theme) const {
        window.draw(theme.makePanel(canvasBounds().position, canvasBounds().size));
    }

    void drawButton(sf::RenderWindow& window, Theme& theme, Renderer& renderer,
                    sf::FloatRect bounds, const string& label) const {
        window.draw(theme.makeButton(bounds.position, bounds.size));
        renderer.drawText(label, bounds.position + bounds.size / 2.f, 14, Palette::TEXT, true);
    }

    void drawHud(sf::RenderWindow& window, Renderer& renderer, const ScoreSystem& score,
                 const string& levelId, const string& title, const string& objective,
                 const string& message, bool failed) const {
        renderer.drawText(levelId, {820.f, 60.f}, 18, Palette::TARGET, false);
        renderer.drawText(title, {820.f, 86.f}, 15, Palette::TEXT, false);
        renderer.drawText(objective, {820.f, 118.f}, 13, Palette::TEXT, false);

        alarm.draw(window, renderer, score, {820.f, 160.f}, 230.f);
        renderer.drawText("Score: " + to_string(score.score()), {820.f, 200.f}, 14, Palette::TEXT, false);
        renderer.drawText("Steps: " + to_string(score.steps()) + " / opt " +
                              to_string(score.optimalSteps()),
                          {820.f, 222.f}, 14, Palette::TEXT, false);
        renderer.drawText("Penalties: " + to_string(score.penalties()), {820.f, 244.f},
                          14, Palette::TEXT, false);
        renderer.drawText(message, {820.f, 272.f}, 12,
                          failed ? Palette::VIOLATION : Palette::WARNING, false);
    }

    void drawEventLog(Renderer& renderer, const EventLog& log, int highlightIndex) const {
        eventLog.draw(renderer, log, highlightIndex, {820.f, 340.f}, 230.f, 5);
    }

    void drawHelp(Renderer& renderer, const string& line) const {
        renderer.drawText(line, {550.f, 680.f}, 12, Palette::MUTED, true);
    }

private:
    AlarmMeter alarm;
    EventLogPanel eventLog;
};

}
