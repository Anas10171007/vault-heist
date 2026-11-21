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

struct LevelChrome {
    static sf::FloatRect canvasBounds() { return {{28.f, 28.f}, {740.f, 620.f}}; }
    static sf::FloatRect sidePanelBounds() { return {{788.f, 28.f}, {284.f, 620.f}}; }

    static sf::FloatRect undoButton() { return {{804.f, 448.f}, {120.f, 34.f}}; }
    static sf::FloatRect hintButton() { return {{936.f, 448.f}, {120.f, 34.f}}; }
    static sf::FloatRect replayButton() { return {{804.f, 490.f}, {120.f, 34.f}}; }
    static sf::FloatRect resetButton() { return {{936.f, 490.f}, {120.f, 34.f}}; }
    static sf::FloatRect slowerButton() { return {{804.f, 532.f}, {120.f, 34.f}}; }
    static sf::FloatRect fasterButton() { return {{936.f, 532.f}, {120.f, 34.f}}; }
    static sf::FloatRect exitButton() { return {{804.f, 580.f}, {252.f, 36.f}}; }
    static sf::FloatRect continueButton() { return {{804.f, 580.f}, {252.f, 36.f}}; }
    static sf::FloatRect opButtonA() { return {{804.f, 318.f}, {120.f, 36.f}}; }
    static sf::FloatRect opButtonB() { return {{936.f, 318.f}, {120.f, 36.f}}; }
    static sf::FloatRect opButtonWide() { return {{804.f, 318.f}, {252.f, 36.f}}; }

    void drawSidePanel(sf::RenderWindow& window, Theme& theme) const {
        theme.drawPanel(window, sidePanelBounds());
    }

    void drawCanvasPanel(sf::RenderWindow& window, Theme& theme) const {
        theme.drawPanel(window, canvasBounds());
    }

    void drawButton(sf::RenderWindow& window, Theme& theme, Renderer& /*renderer*/,
                    sf::FloatRect bounds, const string& label, int kind = ButtonKind::SECONDARY) const {
        theme.drawButton(window, theme.getFont(), bounds, label, kind, theme.hovered(window, bounds));
    }

    static int previewStars(const ScoreSystem& score) {
        if (score.isFailed()) return 0;
        int result = 1;
        if (score.steps() <= score.optimalSteps() && score.penalties() == 0) result = 3;
        else if (score.steps() <= static_cast<int>(score.optimalSteps() * 1.25) &&
                 score.penalties() <= 1) {
            result = 2;
        }
        if (score.usedHint() && result > 2) result = 2;
        if (score.steps() == 0 && score.penalties() == 0) result = 3;
        return result;
    }

    void drawHud(sf::RenderWindow& window, Renderer& renderer, Theme& theme,
                 const ScoreSystem& score, const string& levelId, const string& title,
                 const string& objective, const string& message, bool failed) const {
        renderer.drawText(levelId, {804.f, 44.f}, 13, Palette::BRASS, false);
        renderer.drawText(title, {804.f, 64.f}, 18, Palette::TEXT, false);

        string obj = objective;
        if (obj.size() > 34) obj = obj.substr(0, 33) + "…";
        renderer.drawText(obj, {804.f, 92.f}, 12, Palette::MUTED, false);

        alarm.draw(window, renderer, score, {804.f, 128.f}, 252.f, theme.timeSeconds());

        renderer.drawText("SCORE", {804.f, 168.f}, 11, Palette::MUTED, false);
        renderer.drawText(to_string(score.score()), {804.f, 186.f}, 28, Palette::BRASS, false);

        int opt = score.optimalSteps() < 1 ? 1 : score.optimalSteps();
        float stepRatio = static_cast<float>(score.steps()) / static_cast<float>(opt);
        if (stepRatio > 1.4f) stepRatio = 1.4f;
        sf::RectangleShape stepTrack({252.f, 8.f});
        stepTrack.setPosition({804.f, 228.f});
        stepTrack.setFillColor(Palette::BG);
        window.draw(stepTrack);
        sf::RectangleShape stepFill({252.f * (stepRatio > 1.f ? 1.f : stepRatio), 8.f});
        stepFill.setPosition({804.f, 228.f});
        stepFill.setFillColor(stepRatio <= 1.f ? Palette::VISITED : Palette::WARNING);
        window.draw(stepFill);

        renderer.drawText("Steps " + to_string(score.steps()) + " / opt " + to_string(opt),
                          {804.f, 242.f}, 12, Palette::TEXT, false);
        renderer.drawText("Penalties " + to_string(score.penalties()), {804.f, 260.f}, 12,
                          score.penalties() > 0 ? Palette::WARNING : Palette::MUTED, false);

        int stars = previewStars(score);
        string starLine;
        for (int i = 0; i < 3; i++) starLine += i < stars ? "* " : ". ";
        renderer.drawText(starLine, {804.f, 282.f}, 18, Palette::BRASS, false);
        renderer.drawText(failed ? "FAILED" : "on track", {930.f, 286.f}, 11,
                          failed ? Palette::VIOLATION : Palette::MUTED, false);

        string msg = message;
        if (msg.size() > 38) msg = msg.substr(0, 37) + "…";
        renderer.drawText(msg, {804.f, 368.f}, 12,
                          failed ? Palette::VIOLATION : Palette::WARNING, false);
    }

    void drawEventLog(Renderer& renderer, const EventLog& log, int highlightIndex) const {
        eventLog.draw(renderer, log, highlightIndex, {804.f, 392.f}, 252.f, 3);
    }

    void drawHelp(Renderer& renderer, const string& line) const {
        renderer.drawText(line, {400.f, 668.f}, 12, Palette::MUTED, true);
    }

private:
    AlarmMeter alarm;
    EventLogPanel eventLog;
};

}
