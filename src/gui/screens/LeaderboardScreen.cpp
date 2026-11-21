#include "gui/screens/LeaderboardScreen.h"
#include "gui/core/Renderer.h"
#include "gui/core/ScreenId.h"

namespace vh {

LeaderboardScreen::LeaderboardScreen(Theme& sharedTheme, GameSession& sharedSession)
    : theme(sharedTheme), session(sharedSession) {}

sf::FloatRect LeaderboardScreen::backButton() const {
    return {{350.f, 580.f}, {400.f, 44.f}};
}

void LeaderboardScreen::onShow() {
    board.load();
}

int LeaderboardScreen::update(float) { return ScreenId::NONE; }

int LeaderboardScreen::handleEvent(const sf::Event& event, sf::RenderWindow& window) {
    const auto* key = event.getIf<sf::Event::KeyPressed>();
    if (key && key->code == sf::Keyboard::Key::Escape) return ScreenId::MENU;
    const auto* click = event.getIf<sf::Event::MouseButtonPressed>();
    if (!click || click->button != sf::Mouse::Button::Left) return ScreenId::NONE;
    sf::Vector2f point = window.mapPixelToCoords(click->position);
    if (backButton().contains(point)) return ScreenId::MENU;
    return ScreenId::NONE;
}

void LeaderboardScreen::draw(sf::RenderWindow& window) {
    theme.drawBackdrop(window);
    Renderer renderer(window, theme.getFont());
    theme.drawPanel(window, {{220.f, 50.f}, {660.f, 600.f}});

    renderer.drawText("LEADERBOARD", {550.f, 90.f}, 34, Palette::BRASS, true);
    renderer.drawText("Local top scores", {550.f, 135.f}, 14, Palette::MUTED, true);

    if (board.size() == 0) {
        renderer.drawText("No scores yet. Finish a mission to post one.", {550.f, 300.f}, 16,
                          Palette::MUTED, true);
    } else {
        for (int i = 0; i < board.size() && i < 10; i++) {
            const LeaderboardEntry& entry = board.at(i);
            string line = to_string(i + 1) + ".  " + entry.name + "   " + to_string(entry.score);
            renderer.drawText(line, {550.f, 180.f + i * 34.f}, 18, Palette::TEXT, true);
        }
    }

    theme.drawButton(window, theme.getFont(), backButton(), "Main Menu", ButtonKind::GHOST,
                     theme.hovered(window, backButton()));
}

}
