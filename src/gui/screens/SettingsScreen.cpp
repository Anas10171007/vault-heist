#include "gui/screens/SettingsScreen.h"
#include "gui/core/Renderer.h"
#include "gui/core/ScreenId.h"

namespace vh {
SettingsScreen::SettingsScreen(Theme& t, GameSession& s) : theme(t), session(s) {}
sf::FloatRect SettingsScreen::speedButton() const { return {{350.f, 280.f}, {400.f, 48.f}}; }
sf::FloatRect SettingsScreen::helpButton() const { return {{350.f, 350.f}, {400.f, 48.f}}; }
sf::FloatRect SettingsScreen::backButton() const { return {{350.f, 480.f}, {400.f, 44.f}}; }

int SettingsScreen::handleEvent(const sf::Event& event, sf::RenderWindow& window) {
    const auto* click = event.getIf<sf::Event::MouseButtonPressed>();
    if (!click || click->button != sf::Mouse::Button::Left) return ScreenId::NONE;
    sf::Vector2f point = window.mapPixelToCoords(click->position);
    if (speedButton().contains(point)) {
        session.animationSpeed = session.animationSpeed < 1.f ? 1.f
                                 : (session.animationSpeed < 2.f ? 2.f : 0.5f);
    } else if (helpButton().contains(point)) {
        session.showHelp = !session.showHelp;
    } else if (backButton().contains(point)) {
        return ScreenId::MENU;
    }
    return ScreenId::NONE;
}

void SettingsScreen::draw(sf::RenderWindow& window) {
    theme.drawVaultBackdrop(window);
    Renderer renderer(window, theme.getFont());
    theme.drawPanel(window, {{220.f, 70.f}, {660.f, 520.f}});

    theme.drawGlowTitle(window, "SETTINGS", {550.f, 110.f}, 32, Palette::BRASS, true);
    renderer.drawText("Tune the vault to your pace", {550.f, 158.f}, 13, Palette::MUTED, true);
    theme.drawChip(window, {550.f, 205.f}, "PREFERENCES", 10, Palette::VISITED);

    string speed = session.animationSpeed == 0.5f ? "0.5x"
                   : (session.animationSpeed == 2.f ? "2x" : "1x");
    theme.drawGlossyButton(window, theme.getFont(), speedButton(), "Animation speed",
                           speed + " · click to cycle", ButtonKind::SECONDARY,
                           theme.hovered(window, speedButton()));
    theme.drawGlossyButton(window, theme.getFont(), helpButton(), "Control hints",
                           string(session.showHelp ? "On" : "Off") + " · show tips in missions",
                           ButtonKind::SECONDARY, theme.hovered(window, helpButton()));

    renderer.drawText("Changes apply immediately.", {550.f, 420.f}, 11, Palette::MUTED, true);

    theme.drawGlossyButton(window, theme.getFont(), backButton(), "Main Menu",
                           "back to the lobby", ButtonKind::GHOST,
                           theme.hovered(window, backButton()));
}
}
