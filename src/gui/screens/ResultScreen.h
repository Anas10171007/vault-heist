#pragma once
#include "game/GameSession.h"
#include "gui/core/Screen.h"
#include "gui/core/Theme.h"

namespace vh {
class ResultScreen : public Screen {
public:
    ResultScreen(Theme& theme, GameSession& session);
    int handleEvent(const sf::Event& event, sf::RenderWindow& window) override;
    void update(float) override {}
    void draw(sf::RenderWindow& window) override;
private:
    Theme& theme;
    GameSession& session;
    sf::FloatRect retryButton() const;
    sf::FloatRect nextButton() const;
    sf::FloatRect mapButton() const;
};
}
