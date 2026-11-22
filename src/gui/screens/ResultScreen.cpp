#include "gui/screens/ResultScreen.h"
#include "gui/core/Renderer.h"
#include "gui/core/ScreenId.h"

using namespace std;

namespace vh {
ResultScreen::ResultScreen(Theme& sharedTheme, GameSession& sharedSession)
    : theme(sharedTheme), session(sharedSession) {}

sf::FloatRect ResultScreen::replayButton() const { return {{350.f, 430.f}, {400.f, 40.f}}; }
sf::FloatRect ResultScreen::retryButton() const { return {{350.f, 480.f}, {400.f, 44.f}}; }
sf::FloatRect ResultScreen::nextButton() const { return {{350.f, 534.f}, {400.f, 48.f}}; }
sf::FloatRect ResultScreen::mapButton() const { return {{350.f, 592.f}, {400.f, 40.f}}; }

int ResultScreen::handleEvent(const sf::Event& event, sf::RenderWindow& window) {
    const auto* key = event.getIf<sf::Event::KeyPressed>();
    if (key && key->code == sf::Keyboard::Key::Escape) return ScreenId::CAMPAIGN_MAP;
    const auto* click = event.getIf<sf::Event::MouseButtonPressed>();
    if (!click || click->button != sf::Mouse::Button::Left) return ScreenId::NONE;
    sf::Vector2f point = window.mapPixelToCoords(click->position);
    if (session.hasLevelReplay && replayButton().contains(point)) {
        session.reviewReplay = true;
        return ScreenId::LEVEL_PLAY;
    }
    if (retryButton().contains(point)) {
        session.hasLevelReplay = false;
        session.reviewReplay = false;
        return ScreenId::LEVEL_PLAY;
    }
    if (nextButton().contains(point) && !session.nextLevelId.empty()) {
        session.hasLevelReplay = false;
        session.reviewReplay = false;
        session.selectedLevel = session.nextLevelId;
        return ScreenId::BRIEFING;
    }
    if (mapButton().contains(point)) return ScreenId::CAMPAIGN_MAP;
    return ScreenId::NONE;
}

void ResultScreen::draw(sf::RenderWindow& window) {
    theme.drawVaultBackdrop(window);
    Renderer renderer(window, theme.getFont());
    theme.drawPanel(window, {{220.f, 40.f}, {660.f, 620.f}});

    renderer.drawText("MISSION RESULT", {550.f, 70.f}, 11, Palette::MUTED, true);

    string headline = "Busted. Alarm went off.";
    sf::Color headlineColor = Palette::VIOLATION;
    if (session.stars >= 3) {
        headline = "Flawless. Clean getaway.";
        headlineColor = Palette::BRASS;
    } else if (session.stars == 2) {
        headline = "Not bad, thief.";
        headlineColor = Palette::VISITED;
    } else if (session.stars == 1) {
        headline = "Made it out. Barely.";
        headlineColor = Palette::TEXT;
    }
    theme.drawGlowTitle(window, headline, {550.f, 106.f}, 26, headlineColor, true);
    renderer.drawText(theme.elide(session.resultMessage, 11, 600.f), {550.f, 138.f}, 11,
                      Palette::MUTED, true);

    for (int i = 0; i < 3; i++) {
        theme.drawStar(window, {550.f + static_cast<float>(i - 1) * 40.f, 168.f}, 13.f,
                       i < session.stars);
    }

    string grade = "CLEAR";
    sf::Color gradeColor = Palette::VISITED;
    if (session.stars >= 3) {
        grade = "PERFECT";
        gradeColor = Palette::BRASS;
    } else if (session.stars == 2) {
        grade = "CLEAN";
    } else if (session.stars <= 0) {
        grade = "FAILED";
        gradeColor = Palette::VIOLATION;
    }
    theme.drawChip(window, {550.f, 205.f}, grade, 10, gradeColor);

    theme.drawCard(window, {{300.f, 238.f}, {220.f, 78.f}}, Palette::BRASS, false);
    renderer.drawText("SCORE", {410.f, 252.f}, 10, Palette::MUTED, true);
    renderer.drawText(to_string(session.score), {410.f, 278.f}, 30, Palette::BRASS, true);

    theme.drawCard(window, {{580.f, 238.f}, {220.f, 78.f}}, Palette::VISITED, false);
    renderer.drawText("STEPS / OPTIMAL", {690.f, 252.f}, 10, Palette::MUTED, true);
    renderer.drawText(to_string(session.steps) + " / " + to_string(session.optimalSteps),
                      {690.f, 278.f}, 20, Palette::TEXT, true);

    sf::RectangleShape rule({560.f, 1.f});
    rule.setPosition({270.f, 332.f});
    rule.setFillColor(Palette::BORDER);
    window.draw(rule);

    renderer.drawText("Penalties  " + to_string(session.penalties), {550.f, 350.f}, 14,
                      session.penalties > 0 ? Palette::WARNING : Palette::MUTED, true);

    int opt = session.optimalSteps < 1 ? 1 : session.optimalSteps;
    int efficiency = 100;
    if (session.steps > 0) efficiency = (opt * 100) / session.steps;
    if (efficiency > 100) efficiency = 100;
    renderer.drawText("Efficiency  " + to_string(efficiency) + "%", {550.f, 376.f}, 14,
                      Palette::TEXT, true);

    if (session.finalVaultUnlocked && !session.hasLevelReplay) {
        renderer.drawText("FINAL VAULT UNLOCKED", {550.f, 402.f}, 13, Palette::BRASS, true);
        if (!session.sortedArrayText.empty()) {
            string sorted = session.sortedArrayText;
            if (sorted.size() > 48) sorted = sorted.substr(0, 47) + "…";
            renderer.drawText(sorted, {550.f, 418.f}, 10, Palette::MUTED, true);
        }
    }

    if (session.hasLevelReplay) {
        theme.drawGlossyButton(window, theme.getFont(), replayButton(), "Replay Run",
                               ButtonKind::PRIMARY, theme.hovered(window, replayButton()));
    }
    theme.drawGlossyButton(window, theme.getFont(), retryButton(), "Retry",
                           ButtonKind::SECONDARY, theme.hovered(window, retryButton()));
    if (!session.nextLevelId.empty()) {
        theme.drawGlossyButton(window, theme.getFont(), nextButton(), "Next Level",
                               ButtonKind::PRIMARY, theme.hovered(window, nextButton()));
    }
    theme.drawGlossyButton(window, theme.getFont(), mapButton(), "Campaign Map",
                           ButtonKind::GHOST, theme.hovered(window, mapButton()));
}
}
