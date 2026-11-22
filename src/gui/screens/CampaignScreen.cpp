#include "gui/screens/CampaignScreen.h"
#include "gui/core/Renderer.h"
#include "gui/core/ScreenId.h"

using namespace std;

namespace vh {

CampaignScreen::CampaignScreen(Theme& sharedTheme, GameSession& sharedSession)
    : theme(sharedTheme), session(sharedSession), loaded(false) {
    const char* paths[] = {"data/levels", "../data/levels", "../../data/levels"};
    for (const char* path : paths) if (levels.loadAll(path)) { loaded = true; break; }
}

sf::FloatRect CampaignScreen::levelBounds(int index) const {
    int act = levels.at(index).act;
    if (act < 1) act = 1;
    if (act > 4) act = 4;
    int slot = 0;
    for (int i = 0; i < index; i++) {
        if (levels.at(i).act == act) slot++;
    }
    float col = 70.f + (act - 1) * 255.f;
    float row = 150.f + slot * 52.f;
    return {{col, row}, {236.f, 44.f}};
}

bool CampaignScreen::isUnlocked(int index) const {
    if (index <= 0) return true;
    const string& previousId = levels.at(index - 1).id;
    return session.bestStarsFor(previousId) > 0;
}

int CampaignScreen::handleEvent(const sf::Event& event, sf::RenderWindow& window) {
    const auto* key = event.getIf<sf::Event::KeyPressed>();
    if (key && (key->code == sf::Keyboard::Key::Backspace ||
                key->code == sf::Keyboard::Key::Escape)) {
        return ScreenId::MENU;
    }
    const auto* click = event.getIf<sf::Event::MouseButtonPressed>();
    if (!click || click->button != sf::Mouse::Button::Left || !loaded) return ScreenId::NONE;
    sf::Vector2f point = window.mapPixelToCoords(click->position);
    for (int index = 0; index < levels.size(); index++) {
        if (!levelBounds(index).contains(point)) continue;
        if (!isUnlocked(index)) return ScreenId::NONE;
        session.selectedLevel = levels.at(index).id;
        return ScreenId::BRIEFING;
    }
    return ScreenId::NONE;
}

void CampaignScreen::draw(sf::RenderWindow& window) {
    theme.drawVaultBackdrop(window);
    Renderer renderer(window, theme.getFont());
    theme.drawPanel(window, {{40.f, 28.f}, {1020.f, 640.f}});

    theme.drawGlowTitle(window, "CAMPAIGN", {550.f, 52.f}, 32, Palette::BRASS, true);
    renderer.drawText("Clear a mission to unlock the next  ·  Esc / Backspace: menu",
                      {550.f, 92.f}, 12, Palette::MUTED, true);

    if (!loaded) {
        renderer.drawText("Could not load level data", {550.f, 330.f}, 20, Palette::VIOLATION, true);
        return;
    }

    const char* actNames[4] = {"ACT 1 · BINARY BLUEPRINT", "ACT 2 · BST VAULT",
                               "ACT 3 · AVL FORTRESS", "ACT 4 · HEAP FORTRESS"};
    const char* actBlurbs[4] = {"read trees every which way", "sorted rooms, no duplicates",
                                "balance or bust", "priority queue, heist edition"};
    sf::Color actAccents[4] = {sf::Color(64, 200, 190), sf::Color(88, 196, 132),
                               Palette::BRASS, sf::Color(232, 142, 84)};
    for (int act = 1; act <= 4; act++) {
        float x = 70.f + (act - 1) * 255.f;
        renderer.drawText(actNames[act - 1], {x + 118.f, 118.f}, 11, actAccents[act - 1], true);
        renderer.drawText(actBlurbs[act - 1], {x + 118.f, 133.f}, 9, Palette::MUTED, true);
    }

    for (int index = 0; index < levels.size(); index++) {
        const LevelDef& level = levels.at(index);
        sf::FloatRect bounds = levelBounds(index);
        bool unlocked = isUnlocked(index);
        bool hovered = unlocked && theme.hovered(window, bounds);
        int best = session.bestStarsFor(level.id);
        int act = level.act < 1 ? 1 : (level.act > 4 ? 4 : level.act);
        sf::Color accent = actAccents[act - 1];
        theme.drawCard(window, bounds, accent, hovered);

        renderer.drawText(level.id, bounds.position + sf::Vector2f(11.f, 5.f), 10, accent,
                          false);
        for (int s = 0; s < 3; s++) {
            theme.drawStar(window,
                           {bounds.position.x + 206.f + static_cast<float>(s) * 12.f,
                            bounds.position.y + 12.f},
                           4.5f, s < best);
        }

        string title = theme.elide(level.title, 11, 210.f);
        if (!unlocked) title = "LOCKED";
        renderer.drawText(title, bounds.position + sf::Vector2f(11.f, 21.f), 11,
                          unlocked ? Palette::TEXT : Palette::MUTED, false);
    }

    renderer.drawText("Gold stars mark your best run  ·  clear a mission to open the next",
                      {550.f, 620.f}, 11, Palette::MUTED, true);
}

}
