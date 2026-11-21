#include "gui/screens/BriefingScreen.h"
#include "gui/core/Renderer.h"
#include "gui/core/ScreenId.h"

namespace vh {
BriefingScreen::BriefingScreen(Theme& sharedTheme, GameSession& sharedSession)
    : theme(sharedTheme), session(sharedSession) {
    const char* paths[] = {"data/levels", "../data/levels", "../../data/levels"};
    for (const char* path : paths) if (levels.loadAll(path)) break;
}
sf::FloatRect BriefingScreen::startButton() const { return {{350.f, 445.f}, {400.f, 48.f}}; }
sf::FloatRect BriefingScreen::backButton() const { return {{350.f, 510.f}, {400.f, 48.f}}; }
int BriefingScreen::handleEvent(const sf::Event& event, sf::RenderWindow& window) {
    const auto* key = event.getIf<sf::Event::KeyPressed>();
    if (key && key->code == sf::Keyboard::Key::Enter) return ScreenId::LEVEL_PLAY;
    const auto* click = event.getIf<sf::Event::MouseButtonPressed>();
    if (!click || click->button != sf::Mouse::Button::Left) return ScreenId::NONE;
    sf::Vector2f point = window.mapPixelToCoords(click->position);
    if (startButton().contains(point)) return ScreenId::LEVEL_PLAY;
    if (backButton().contains(point)) return ScreenId::CAMPAIGN_MAP;
    return ScreenId::NONE;
}
void BriefingScreen::draw(sf::RenderWindow& window) {
    Renderer renderer(window, theme.getFont());
    window.draw(theme.makePanel({220.f, 75.f}, {660.f, 550.f}));
    const LevelDef* level = levels.load(session.selectedLevel);
    if (!level) { renderer.drawText("LEVEL DATA UNAVAILABLE", {550.f, 250.f}, 24, Palette::VIOLATION, true); return; }
    renderer.drawText("MISSION " + level->id, {550.f, 120.f}, 20, Palette::TARGET, true);
    renderer.drawText(level->title, {550.f, 165.f}, 32, Palette::TEXT, true);
    renderer.drawText(level->mission, {550.f, 235.f}, 16, Palette::MUTED, true);
    renderer.drawText("Structure: " + level->structure, {550.f, 300.f}, 17, Palette::TEXT, true);
    renderer.drawText("Task: " + level->task, {550.f, 332.f}, 17, Palette::TEXT, true);
    window.draw(theme.makeButton(startButton().position, startButton().size));
    renderer.drawText("Start Mission  [Enter]", startButton().position + startButton().size / 2.f, 19, Palette::TEXT, true);
    window.draw(theme.makeButton(backButton().position, backButton().size));
    renderer.drawText("Back to Campaign", backButton().position + backButton().size / 2.f, 19, Palette::TEXT, true);
}
}
