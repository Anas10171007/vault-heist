#include "gui/screens/CampaignScreen.h"
#include "gui/core/Renderer.h"
#include "gui/core/ScreenId.h"

namespace vh {

CampaignScreen::CampaignScreen(Theme& sharedTheme, GameSession& sharedSession)
    : theme(sharedTheme), session(sharedSession), loaded(false) {
    const char* paths[] = {"data/levels", "../data/levels", "../../data/levels"};
    for (const char* path : paths) if (levels.loadAll(path)) { loaded = true; break; }
}

sf::FloatRect CampaignScreen::levelBounds(int index) const {
    int column = index / 10;
    int row = index % 10;
    return {{105.f + column * 470.f, 115.f + row * 50.f}, {420.f, 40.f}};
}

int CampaignScreen::handleEvent(const sf::Event& event, sf::RenderWindow& window) {
    const auto* key = event.getIf<sf::Event::KeyPressed>();
    if (key && key->code == sf::Keyboard::Key::Backspace) return ScreenId::MENU;
    const auto* click = event.getIf<sf::Event::MouseButtonPressed>();
    if (!click || click->button != sf::Mouse::Button::Left || !loaded) return ScreenId::NONE;
    sf::Vector2f point = window.mapPixelToCoords(click->position);
    for (int index = 0; index < levels.size(); index++) {
        if (levelBounds(index).contains(point)) {
            session.selectedLevel = levels.at(index).id;
            return ScreenId::BRIEFING;
        }
    }
    return ScreenId::NONE;
}

void CampaignScreen::draw(sf::RenderWindow& window) {
    Renderer renderer(window, theme.getFont());
    window.draw(theme.makePanel({70.f, 45.f}, {960.f, 610.f}));
    renderer.drawText("CAMPAIGN MAP", {550.f, 70.f}, 30, Palette::TARGET, true);
    if (!loaded) {
        renderer.drawText("Could not load level data", {550.f, 330.f}, 20, Palette::VIOLATION, true);
        return;
    }
    for (int index = 0; index < levels.size(); index++) {
        const LevelDef& level = levels.at(index);
        sf::FloatRect bounds = levelBounds(index);
        window.draw(theme.makeButton(bounds.position, bounds.size));
        renderer.drawText(level.id + "  " + level.title, bounds.position + sf::Vector2f(14.f, 10.f),
                          15, Palette::TEXT, false);
    }
    renderer.drawText("Select any of the 20 missions  |  Backspace: menu",
                      {550.f, 625.f}, 14, Palette::MUTED, true);
}

}
