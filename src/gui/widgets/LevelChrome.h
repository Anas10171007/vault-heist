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
    static sf::FloatRect canvasBounds() { return {{24.f, 24.f}, {748.f, 628.f}}; }
    static sf::FloatRect sidePanelBounds() { return {{792.f, 24.f}, {284.f, 628.f}}; }

    // Contextual ops — one row, only when needed
    static sf::FloatRect opButtonA() { return {{808.f, 236.f}, {120.f, 36.f}}; }
    static sf::FloatRect opButtonB() { return {{940.f, 236.f}, {120.f, 36.f}}; }
    static sf::FloatRect opButtonC() { return {{808.f, 236.f}, {252.f, 36.f}}; }
    static sf::FloatRect opButtonD() { return {{940.f, 236.f}, {120.f, 36.f}}; }
    static sf::FloatRect opButtonWide() { return {{808.f, 236.f}, {252.f, 36.f}}; }

    // Toolbar — lower third, spaced so the rail can breathe
    static sf::FloatRect undoButton() { return {{808.f, 420.f}, {120.f, 34.f}}; }
    static sf::FloatRect hintButton() { return {{940.f, 420.f}, {120.f, 34.f}}; }
    static sf::FloatRect replayButton() { return {{808.f, 468.f}, {120.f, 34.f}}; }
    static sf::FloatRect resetButton() { return {{940.f, 468.f}, {120.f, 34.f}}; }
    static sf::FloatRect slowerButton() { return {{808.f, 516.f}, {120.f, 34.f}}; }
    static sf::FloatRect fasterButton() { return {{940.f, 516.f}, {120.f, 34.f}}; }
    static sf::FloatRect exitButton() { return {{808.f, 580.f}, {252.f, 40.f}}; }
    static sf::FloatRect continueButton() { return {{808.f, 580.f}, {252.f, 40.f}}; }

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

    static string clip(const string& text, int maxChars) {
        if (static_cast<int>(text.size()) <= maxChars) return text;
        if (maxChars < 2) return text;
        return text.substr(0, maxChars - 1) + "…";
    }

    void drawHud(sf::RenderWindow& window, Renderer& renderer, Theme& theme,
                 const ScoreSystem& score, const string& levelId, const string& title,
                 const string& /*objective*/, const string& message, bool failed) const {
        renderer.drawText(levelId, {808.f, 40.f}, 12, Palette::BRASS, false);
        renderer.drawText(clip(title, 22), {808.f, 58.f}, 16, Palette::TEXT, false);

        alarm.draw(window, renderer, score, {808.f, 92.f}, 252.f, theme.timeSeconds());

        renderer.drawText("SCORE", {808.f, 138.f}, 11, Palette::MUTED, false);
        renderer.drawText(to_string(score.score()), {808.f, 154.f}, 26, Palette::BRASS, false);

        int opt = score.optimalSteps() < 1 ? 1 : score.optimalSteps();
        string meta = "Steps " + to_string(score.steps()) + "/" + to_string(opt) +
                      "   Pen " + to_string(score.penalties());
        renderer.drawText(meta, {808.f, 188.f}, 12, Palette::MUTED, false);

        int stars = previewStars(score);
        string starLine;
        for (int i = 0; i < 3; i++) starLine += i < stars ? "* " : ". ";
        renderer.drawText(starLine, {808.f, 208.f}, 16, Palette::BRASS, false);
        renderer.drawText(failed ? "FAILED" : "on track", {960.f, 210.f}, 11,
                          failed ? Palette::VIOLATION : Palette::MUTED, false);

        // Status under the action slot — wrap to two lines so tips are not cut off.
        sf::Color statusColor = failed ? Palette::VIOLATION : Palette::WARNING;
        string rest = message;
        float y = 278.f;
        for (int line = 0; line < 2 && !rest.empty(); line++) {
            string piece;
            if (static_cast<int>(rest.size()) <= 36) {
                piece = rest;
                rest.clear();
            } else if (line == 1) {
                piece = clip(rest, 36);
                rest.clear();
            } else {
                int cut = 36;
                for (int i = 35; i >= 18; i--) {
                    if (rest[static_cast<size_t>(i)] == ' ') {
                        cut = i;
                        break;
                    }
                }
                piece = rest.substr(0, static_cast<size_t>(cut));
                rest = rest.substr(static_cast<size_t>(cut));
                while (!rest.empty() && rest[0] == ' ') rest.erase(0, 1);
            }
            renderer.drawText(piece, {808.f, y}, 12, statusColor, false);
            y += 16.f;
        }
    }

    void drawEventLog(Renderer& renderer, const EventLog& log, int highlightIndex) const {
        // Stay quiet until the model actually emits something.
        if (log.empty()) return;
        eventLog.draw(renderer, log, highlightIndex, {808.f, 318.f}, 252.f, 3);
    }

    void drawHelp(Renderer& /*renderer*/, const string& /*line*/) const {
        // Help lives in the status line now — keep footer clear.
    }

private:
    AlarmMeter alarm;
    EventLogPanel eventLog;
};

}
