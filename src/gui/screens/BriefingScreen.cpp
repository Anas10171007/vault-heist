#include "gui/screens/BriefingScreen.h"
#include "gui/core/Renderer.h"
#include "gui/core/ScreenId.h"
#include "gui/screens/TreeMissionScreen.h"

namespace vh {
BriefingScreen::BriefingScreen(Theme& sharedTheme, GameSession& sharedSession)
    : theme(sharedTheme), session(sharedSession) {
    const char* paths[] = {"data/levels", "../data/levels", "../../data/levels"};
    for (const char* path : paths) if (levels.loadAll(path)) break;
}
sf::FloatRect BriefingScreen::startButton() const { return {{350.f, 470.f}, {400.f, 48.f}}; }
sf::FloatRect BriefingScreen::backButton() const { return {{350.f, 532.f}, {400.f, 44.f}}; }

int BriefingScreen::handleEvent(const sf::Event& event, sf::RenderWindow& window) {
    const auto* key = event.getIf<sf::Event::KeyPressed>();
    if (key && key->code == sf::Keyboard::Key::Escape) return ScreenId::CAMPAIGN_MAP;
    if (key && key->code == sf::Keyboard::Key::Enter) return ScreenId::LEVEL_PLAY;
    const auto* click = event.getIf<sf::Event::MouseButtonPressed>();
    if (!click || click->button != sf::Mouse::Button::Left) return ScreenId::NONE;
    sf::Vector2f point = window.mapPixelToCoords(click->position);
    if (startButton().contains(point)) return ScreenId::LEVEL_PLAY;
    if (backButton().contains(point)) return ScreenId::CAMPAIGN_MAP;
    return ScreenId::NONE;
}

void BriefingScreen::draw(sf::RenderWindow& window) {
    theme.drawBackdrop(window);
    Renderer renderer(window, theme.getFont());
    theme.drawPanel(window, {{200.f, 60.f}, {700.f, 560.f}});

    const LevelDef* level = levels.load(session.selectedLevel);
    if (!level) {
        renderer.drawText("LEVEL DATA UNAVAILABLE", {550.f, 250.f}, 24, Palette::VIOLATION, true);
        return;
    }

    renderer.drawText("MISSION " + level->id, {550.f, 100.f}, 14, Palette::BRASS, true);
    renderer.drawText(level->title, {550.f, 140.f}, 34, Palette::TEXT, true);

    sf::RectangleShape rule({160.f, 2.f});
    rule.setPosition({470.f, 188.f});
    rule.setFillColor(Palette::VISITED);
    window.draw(rule);

    string objective = TreeMissionScreen::objectiveFor(*level);
    renderer.drawText(objective, {550.f, 214.f}, 16, Palette::TEXT, true);
    if (objective != level->mission) {
        renderer.drawText(level->mission, {550.f, 246.f}, 13, Palette::MUTED, true);
    }
    renderer.drawText("Structure   " + level->structure, {550.f, 290.f}, 16, Palette::TEXT, true);
    renderer.drawText("Operation   " + level->task, {550.f, 322.f}, 16, Palette::TEXT, true);

    int best = session.bestStarsFor(level->id);
    string stars;
    for (int i = 0; i < 3; i++) stars += i < best ? "* " : ". ";
    renderer.drawText("Best  " + stars, {550.f, 370.f}, 18, Palette::BRASS, true);
    renderer.drawText("3 stars: optimal steps, no penalties, no hints", {550.f, 410.f}, 13,
                      Palette::MUTED, true);

    theme.drawButton(window, theme.getFont(), startButton(), "Start Mission  [Enter]",
                     ButtonKind::PRIMARY, theme.hovered(window, startButton()));
    theme.drawButton(window, theme.getFont(), backButton(), "Back to Campaign",
                     ButtonKind::GHOST, theme.hovered(window, backButton()));
}
}
