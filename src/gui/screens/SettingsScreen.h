#pragma once
#include "game/GameSession.h"
#include "gui/core/Screen.h"
#include "gui/core/Theme.h"
namespace vh {
class SettingsScreen : public Screen {
public:
    SettingsScreen(Theme& theme, GameSession& session);
    int handleEvent(const sf::Event& event, sf::RenderWindow& window) override;
    void update(float) override {}
    void draw(sf::RenderWindow& window) override;
private:
    Theme& theme; GameSession& session;
    sf::FloatRect speedButton() const; sf::FloatRect helpButton() const; sf::FloatRect backButton() const;
};
}
