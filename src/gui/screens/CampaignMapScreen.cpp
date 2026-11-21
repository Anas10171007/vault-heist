#include "gui/screens/CampaignMapScreen.h"

#include <string>

#include "gui/core/Renderer.h"
#include "gui/core/Style.h"
#include "gui/screens/LevelPlayScreen.h"

namespace vh {

CampaignMapScreen::CampaignMapScreen(Theme& sharedTheme, LevelPlayScreen* levelScreen,
                                     SessionProgress& sharedProgress)
    : theme(sharedTheme), play(levelScreen), progress(sharedProgress) {
    manager.loadAll("data/levels");
}

sf::FloatRect CampaignMapScreen::levelBounds(int index) const {
    int act = index / 5;
    int row = index % 5;
    return sf::FloatRect({40.f + static_cast<float>(act) * 265.f, 170.f + static_cast<float>(row) * 78.f},
                         {225.f, 60.f});
}

int CampaignMapScreen::handleEvent(const sf::Event& event, sf::RenderWindow& window) {
    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        if (key->code == sf::Keyboard::Key::Escape) return ScreenId::MENU;
    }
    const auto* click = event.getIf<sf::Event::MouseButtonPressed>();
    if (click == nullptr || click->button != sf::Mouse::Button::Left || play == nullptr) {
        return ScreenId::NONE;
    }
    sf::Vector2f point = window.mapPixelToCoords(click->position);
    for (int i = 0; i < manager.size(); i++) {
        const LevelDef& def = manager.at(i);
        if (def.act >= 4) continue;
        if (!levelBounds(i).contains(point)) continue;
        if (play->loadLevel(def.id)) {
            play->setNextLevel(i + 1 < manager.size() && manager.at(i + 1).act < 4
                                   ? manager.at(i + 1).id
                                   : "");
            return ScreenId::LEVEL_PLAY;
        }
    }
    return ScreenId::NONE;
}

void CampaignMapScreen::update(float deltaSeconds) {
    (void)deltaSeconds;
}

void CampaignMapScreen::onShow() {
    manager.loadAll("data/levels");
}

void CampaignMapScreen::drawAct(Renderer& renderer, sf::RenderTarget& target, int act, float x) {
    const char* names[4] = {"ACT 1 - TREES", "ACT 2 - SEARCH", "ACT 3 - BALANCE",
                            "ACT 4 - HEAP"};
    bool ready = act < 4;
    renderer.drawText(names[act - 1], {x, 122.f}, 17,
                      ready ? Palette::TARGET : Palette::MUTED, false);
    if (!ready) {
        renderer.drawText("COMING SOON", {x + 20.f, 200.f}, 18, Palette::WARNING, false);
    }
    for (int i = 0; i < manager.size(); i++) {
        const LevelDef& def = manager.at(i);
        if (def.act != act) continue;
        sf::FloatRect bounds = levelBounds(i);
        sf::RectangleShape panel(bounds.size);
        panel.setPosition(bounds.position);
        panel.setFillColor(ready ? Palette::PANEL : Palette::BG);
        panel.setOutlineColor(ready ? Palette::NODE_STROKE : Palette::BORDER);
        panel.setOutlineThickness(2.f);
        target.draw(panel);
        renderer.drawText(def.id + "  " + def.title,
                          {bounds.position.x + 12.f, bounds.position.y + 8.f}, 15,
                          ready ? Palette::TEXT : Palette::MUTED, false);
        int stars = progress.starsFor(def.id);
        renderer.drawText(ready ? ("STARS " + std::to_string(stars) + "/3") : "LOCKED",
                          {bounds.position.x + 12.f, bounds.position.y + 32.f}, 13,
                          ready ? (stars == 3 ? Palette::BALANCED : Palette::MUTED) : Palette::MUTED,
                          false);
    }
}

void CampaignMapScreen::drawTo(sf::RenderTarget& target) {
    Renderer renderer(target, theme.getFont());
    renderer.drawText("CAMPAIGN MAP", {40.f, 30.f}, 30, Palette::TARGET, false);
    renderer.drawText("All levels are unlocked. Click a level to play. Esc returns to the menu.",
                      {40.f, 78.f}, 16, Palette::MUTED, false);
    renderer.drawText("TOTAL STARS " + std::to_string(progress.totalStars()),
                      {820.f, 40.f}, 20, Palette::TEXT, false);
    for (int act = 1; act <= 4; act++) {
        drawAct(renderer, target, act, 40.f + static_cast<float>(act - 1) * 265.f);
    }
}

void CampaignMapScreen::draw(sf::RenderWindow& window) {
    drawTo(window);
}

}
