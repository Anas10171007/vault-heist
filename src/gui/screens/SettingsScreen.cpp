#include "gui/screens/SettingsScreen.h"
#include "gui/core/Renderer.h"
#include "gui/core/ScreenId.h"
namespace vh {
SettingsScreen::SettingsScreen(Theme& t, GameSession& s) : theme(t), session(s) {}
sf::FloatRect SettingsScreen::speedButton() const { return {{350.f, 280.f}, {400.f, 50.f}}; }
sf::FloatRect SettingsScreen::helpButton() const { return {{350.f, 350.f}, {400.f, 50.f}}; }
sf::FloatRect SettingsScreen::backButton() const { return {{350.f, 490.f}, {400.f, 50.f}}; }
int SettingsScreen::handleEvent(const sf::Event& event, sf::RenderWindow& window) {
    const auto* click = event.getIf<sf::Event::MouseButtonPressed>();
    if (!click || click->button != sf::Mouse::Button::Left) return ScreenId::NONE;
    sf::Vector2f point = window.mapPixelToCoords(click->position);
    if (speedButton().contains(point)) session.animationSpeed = session.animationSpeed < 1.f ? 1.f : (session.animationSpeed < 2.f ? 2.f : 0.5f);
    else if (helpButton().contains(point)) session.showHelp = !session.showHelp;
    else if (backButton().contains(point)) return ScreenId::MENU;
    return ScreenId::NONE;
}
void SettingsScreen::draw(sf::RenderWindow& window) {
    Renderer renderer(window, theme.getFont());
    window.draw(theme.makePanel({220.f, 75.f}, {660.f, 550.f}));
    renderer.drawText("SETTINGS", {550.f, 120.f}, 34, Palette::TARGET, true);
    renderer.drawText("Gameplay preferences apply to every mission", {550.f, 185.f}, 17, Palette::MUTED, true);
    const sf::FloatRect buttons[] = {speedButton(), helpButton(), backButton()};
    string speed = session.animationSpeed == 0.5f ? "0.5x" : (session.animationSpeed == 2.f ? "2x" : "1x");
    const string labels[] = {"Animation speed: " + speed, "Control hints: " + string(session.showHelp ? "On" : "Off"), "Main Menu"};
    for (int i = 0; i < 3; i++) { window.draw(theme.makeButton(buttons[i].position, buttons[i].size)); renderer.drawText(labels[i], buttons[i].position + buttons[i].size / 2.f, 19, Palette::TEXT, true); }
}
}
