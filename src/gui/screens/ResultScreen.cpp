#include "gui/screens/ResultScreen.h"
#include "gui/core/Renderer.h"
#include "gui/core/ScreenId.h"

using namespace std;

namespace vh {
ResultScreen::ResultScreen(Theme& sharedTheme, GameSession& sharedSession)
    : theme(sharedTheme), session(sharedSession) {}
sf::FloatRect ResultScreen::retryButton() const { return {{350.f, 465.f}, {400.f, 48.f}}; }
sf::FloatRect ResultScreen::mapButton() const { return {{350.f, 530.f}, {400.f, 48.f}}; }
int ResultScreen::handleEvent(const sf::Event& event, sf::RenderWindow& window) {
    const auto* click = event.getIf<sf::Event::MouseButtonPressed>();
    if (!click || click->button != sf::Mouse::Button::Left) return ScreenId::NONE;
    sf::Vector2f point = window.mapPixelToCoords(click->position);
    if (retryButton().contains(point)) return ScreenId::LEVEL_PLAY;
    if (mapButton().contains(point)) return ScreenId::CAMPAIGN_MAP;
    return ScreenId::NONE;
}
void ResultScreen::draw(sf::RenderWindow& window) {
    Renderer renderer(window, theme.getFont());
    window.draw(theme.makePanel({220.f, 75.f}, {660.f, 550.f}));
    renderer.drawText("MISSION RESULT", {550.f, 120.f}, 30, Palette::TARGET, true);
    renderer.drawText(session.resultMessage, {550.f, 180.f}, 20, Palette::TEXT, true);
    string stars;
    for (int i = 0; i < 3; i++) stars += i < session.stars ? "* " : "- ";
    renderer.drawText(stars, {550.f, 230.f}, 34, Palette::WARNING, true);
    renderer.drawText("Score: " + to_string(session.score), {550.f, 290.f}, 20, Palette::TEXT, true);
    renderer.drawText("Steps: " + to_string(session.steps) + "   Penalties: " + to_string(session.penalties),
                      {550.f, 325.f}, 18, Palette::TEXT, true);

    if (session.finalVaultUnlocked) {
        renderer.drawText("Final Vault Unlocked", {550.f, 365.f}, 22, Palette::TARGET, true);
        if (!session.sortedArrayText.empty()) {
            renderer.drawText("Sorted: " + session.sortedArrayText, {550.f, 405.f}, 16,
                              Palette::TEXT, true);
        }
    }

    window.draw(theme.makeButton(retryButton().position, retryButton().size));
    renderer.drawText("Retry", retryButton().position + retryButton().size / 2.f, 19, Palette::TEXT, true);
    window.draw(theme.makeButton(mapButton().position, mapButton().size));
    renderer.drawText("Campaign Map", mapButton().position + mapButton().size / 2.f, 19, Palette::TEXT, true);
}
}
