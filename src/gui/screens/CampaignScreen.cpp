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
    theme.drawBackdrop(window);
    Renderer renderer(window, theme.getFont());
    theme.drawPanel(window, {{40.f, 28.f}, {1020.f, 640.f}});

    renderer.drawText("CAMPAIGN", {550.f, 52.f}, 34, Palette::BRASS, true);
    renderer.drawText("Clear a mission to unlock the next  ·  Backspace: menu",
                      {550.f, 92.f}, 13, Palette::MUTED, true);

    if (!loaded) {
        renderer.drawText("Could not load level data", {550.f, 330.f}, 20, Palette::VIOLATION, true);
        return;
    }

    const char* actNames[4] = {"ACT 1  Binary", "ACT 2  BST", "ACT 3  AVL", "ACT 4  Heap"};
    for (int act = 1; act <= 4; act++) {
        float x = 70.f + (act - 1) * 255.f;
        renderer.drawText(actNames[act - 1], {x + 118.f, 124.f}, 13, Palette::VISITED, true);
    }

    for (int index = 0; index < levels.size(); index++) {
        const LevelDef& level = levels.at(index);
        sf::FloatRect bounds = levelBounds(index);
        bool unlocked = isUnlocked(index);
        int best = session.bestStarsFor(level.id);
        int kind = unlocked ? ButtonKind::SECONDARY : ButtonKind::GHOST;
        if (unlocked && best >= 3) kind = ButtonKind::PRIMARY;
        theme.drawButton(window, theme.getFont(), bounds, "", kind,
                         unlocked && theme.hovered(window, bounds));

        string stars;
        for (int s = 0; s < 3; s++) stars += s < best ? "*" : ".";
        string label = unlocked ? (level.id + "  " + stars) : (level.id + "  LOCK");
        renderer.drawText(label, bounds.position + sf::Vector2f(12.f, 12.f), 14,
                          unlocked ? Palette::TEXT : Palette::MUTED, false);
    }
}

}
