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
    theme.drawBackdrop(window);
    Renderer renderer(window, theme.getFont());
    theme.drawPanel(window, {{220.f, 40.f}, {660.f, 620.f}});

    renderer.drawText("MISSION RESULT", {550.f, 70.f}, 14, Palette::MUTED, true);
    renderer.drawText(session.resultMessage, {550.f, 108.f}, 28, Palette::TEXT, true);

    string stars;
    for (int i = 0; i < 3; i++) stars += i < session.stars ? "*  " : ".  ";
    renderer.drawText(stars, {550.f, 165.f}, 40, Palette::BRASS, true);

    string grade = "CLEAR";
    if (session.stars >= 3) grade = "PERFECT";
    else if (session.stars == 2) grade = "CLEAN";
    else if (session.stars <= 0) grade = "FAILED";
    renderer.drawText(grade, {550.f, 220.f}, 16, Palette::VISITED, true);

    renderer.drawText("SCORE", {360.f, 270.f}, 12, Palette::MUTED, false);
    renderer.drawText(to_string(session.score), {360.f, 292.f}, 36, Palette::BRASS, false);

    renderer.drawText("STEPS", {620.f, 270.f}, 12, Palette::MUTED, false);
    renderer.drawText(to_string(session.steps) + " / " + to_string(session.optimalSteps),
                      {620.f, 298.f}, 20, Palette::TEXT, false);

    renderer.drawText("Penalties  " + to_string(session.penalties), {550.f, 360.f}, 15,
                      session.penalties > 0 ? Palette::WARNING : Palette::MUTED, true);

    int opt = session.optimalSteps < 1 ? 1 : session.optimalSteps;
    int efficiency = 100;
    if (session.steps > 0) efficiency = (opt * 100) / session.steps;
    if (efficiency > 100) efficiency = 100;
    renderer.drawText("Efficiency  " + to_string(efficiency) + "%", {550.f, 390.f}, 15,
                      Palette::TEXT, true);

    if (session.finalVaultUnlocked && !session.hasLevelReplay) {
        renderer.drawText("FINAL VAULT UNLOCKED", {550.f, 418.f}, 14, Palette::BRASS, true);
        if (!session.sortedArrayText.empty()) {
            string sorted = session.sortedArrayText;
            if (sorted.size() > 48) sorted = sorted.substr(0, 47) + "…";
            renderer.drawText(sorted, {550.f, 438.f}, 11, Palette::MUTED, true);
        }
    }

    if (session.hasLevelReplay) {
        theme.drawButton(window, theme.getFont(), replayButton(), "Replay Run",
                         ButtonKind::PRIMARY, theme.hovered(window, replayButton()));
    }
    theme.drawButton(window, theme.getFont(), retryButton(), "Retry", ButtonKind::SECONDARY,
                     theme.hovered(window, retryButton()));
    if (!session.nextLevelId.empty()) {
        theme.drawButton(window, theme.getFont(), nextButton(), "Next Level", ButtonKind::PRIMARY,
                         theme.hovered(window, nextButton()));
    }
    theme.drawButton(window, theme.getFont(), mapButton(), "Campaign Map", ButtonKind::GHOST,
                     theme.hovered(window, mapButton()));
}
}
