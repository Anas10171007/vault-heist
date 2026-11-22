#pragma once

#include "game/LevelManager.h"
#include "gui/core/Renderer.h"
#include "gui/core/Screen.h"
#include "gui/core/ScreenId.h"
#include "gui/core/Theme.h"
#include "gui/screens/SessionProgress.h"

namespace vh {

class LevelPlayScreen;

class CampaignMapScreen : public Screen {
public:
    CampaignMapScreen(Theme& theme, LevelPlayScreen* play, SessionProgress& progress);

    int handleEvent(const sf::Event& event, sf::RenderWindow& window) override;
    int update(float deltaSeconds) override;
    void draw(sf::RenderWindow& window) override;
    void drawTo(sf::RenderTarget& target);
    void onShow() override;

private:
    sf::FloatRect levelBounds(int index) const;
    void drawAct(Renderer& renderer, sf::RenderTarget& target, int act, float x);

    Theme& theme;
    LevelPlayScreen* play;
    SessionProgress& progress;
    LevelManager manager;
};

}
