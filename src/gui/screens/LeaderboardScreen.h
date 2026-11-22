#pragma once
#include "game/GameSession.h"
#include "game/Leaderboard.h"
#include "gui/core/Screen.h"
#include "gui/core/Theme.h"

namespace vh {

class LeaderboardScreen : public Screen {
public:
    LeaderboardScreen(Theme& theme, GameSession& session);
    int handleEvent(const sf::Event& event, sf::RenderWindow& window) override;
    int update(float) override;
    void draw(sf::RenderWindow& window) override;
    void onShow() override;

private:
    Theme& theme;
    GameSession& session;
    Leaderboard board;
    sf::FloatRect backButton() const;
};

}
