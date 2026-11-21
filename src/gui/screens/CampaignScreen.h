#pragma once
#include "game/GameSession.h"
#include "game/LevelManager.h"
#include "gui/core/Screen.h"
#include "gui/core/Theme.h"

namespace vh {

class CampaignScreen : public Screen {
public:
    CampaignScreen(Theme& theme, GameSession& session);
    int handleEvent(const sf::Event& event, sf::RenderWindow& window) override;
    void update(float) override {}
    void draw(sf::RenderWindow& window) override;
private:
    Theme& theme;
    GameSession& session;
    LevelManager levels;
    bool loaded;
    sf::FloatRect levelBounds(int index) const;
    bool isUnlocked(int index) const;
};

}
