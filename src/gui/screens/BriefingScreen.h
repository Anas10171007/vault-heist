#pragma once
#include "game/GameSession.h"
#include "game/LevelManager.h"
#include "gui/core/Screen.h"
#include "gui/core/Theme.h"

namespace vh {
class BriefingScreen : public Screen {
public:
    BriefingScreen(Theme& theme, GameSession& session);
    int handleEvent(const sf::Event& event, sf::RenderWindow& window) override;
    void update(float) override {}
    void draw(sf::RenderWindow& window) override;
private:
    Theme& theme;
    GameSession& session;
    LevelManager levels;
    sf::FloatRect startButton() const;
    sf::FloatRect backButton() const;
};
}
