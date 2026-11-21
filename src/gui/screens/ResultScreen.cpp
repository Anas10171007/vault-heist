#include "gui/screens/ResultScreen.h"
#include "gui/core/Renderer.h"
#include "gui/core/ScreenId.h"

using namespace std;

namespace vh {
ResultScreen::ResultScreen(Theme& sharedTheme, GameSession& sharedSession)
    : theme(sharedTheme), session(sharedSession) {}

sf::FloatRect ResultScreen::retryButton() const { return {{350.f, 400.f}, {400.f, 42.f}}; }
sf::FloatRect ResultScreen::nextButton() const { return {{350.f, 450.f}, {400.f, 42.f}}; }
sf::FloatRect ResultScreen::mapButton() const { return {{350.f, 500.f}, {400.f, 42.f}}; }

int ResultScreen::handleEvent(const sf::Event& event, sf::RenderWindow& window) {
    const auto* click = event.getIf<sf::Event::MouseButtonPressed>();
    if (!click || click->button != sf::Mouse::Button::Left) return ScreenId::NONE;
    sf::Vector2f point = window.mapPixelToCoords(click->position);
    if (retryButton().contains(point)) return ScreenId::LEVEL_PLAY;
    if (nextButton().contains(point) && !session.nextLevelId.empty()) {
        session.selectedLevel = session.nextLevelId;
        return ScreenId::BRIEFING;
    }
    if (mapButton().contains(point)) return ScreenId::CAMPAIGN_MAP;
    return ScreenId::NONE;
}

void ResultScreen::draw(sf::RenderWindow& window) {
    Renderer renderer(window, theme.getFont());
    window.draw(theme.makePanel({220.f, 55.f}, {660.f, 580.f}));
    renderer.drawText("MISSION RESULT", {550.f, 95.f}, 30, Palette::TARGET, true);
    renderer.drawText(session.resultMessage, {550.f, 145.f}, 18, Palette::TEXT, true);

    string stars;
    for (int i = 0; i < 3; i++) stars += i < session.stars ? "* " : "- ";
    renderer.drawText(stars, {550.f, 190.f}, 32, Palette::WARNING, true);

    renderer.drawText("Score: " + to_string(session.score), {550.f, 245.f}, 20, Palette::TEXT, true);
    renderer.drawText("Steps: " + to_string(session.steps) + " / optimal " +
                          to_string(session.optimalSteps),
                      {550.f, 280.f}, 18, Palette::TEXT, true);
    renderer.drawText("Penalties: " + to_string(session.penalties), {550.f, 310.f}, 18,
                      Palette::TEXT, true);

    if (session.finalVaultUnlocked) {
        renderer.drawText("Final Vault Unlocked", {550.f, 350.f}, 20, Palette::TARGET, true);
        if (!session.sortedArrayText.empty()) {
            renderer.drawText("Sorted: " + session.sortedArrayText, {550.f, 380.f}, 14,
                              Palette::TEXT, true);
        }
    }

    window.draw(theme.makeButton(retryButton().position, retryButton().size));
    renderer.drawText("Retry", retryButton().position + retryButton().size / 2.f, 18,
                      Palette::TEXT, true);

    if (!session.nextLevelId.empty()) {
        window.draw(theme.makeButton(nextButton().position, nextButton().size));
        renderer.drawText("Next Level", nextButton().position + nextButton().size / 2.f, 18,
                          Palette::TEXT, true);
    }

    window.draw(theme.makeButton(mapButton().position, mapButton().size));
    renderer.drawText("Campaign Map", mapButton().position + mapButton().size / 2.f, 18,
                      Palette::TEXT, true);
}
}
