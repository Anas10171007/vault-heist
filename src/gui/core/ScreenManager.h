#pragma once
#include "gui/core/Screen.h"
#include "gui/core/ScreenId.h"
#include "gui/core/Theme.h"
#include "game/GameSession.h"

namespace vh {

class ScreenManager {
public:
    ScreenManager(Theme& theme);
    ~ScreenManager();
    void handleEvent(const sf::Event& event, sf::RenderWindow& window);
    void update(float deltaSeconds);
    void draw(sf::RenderWindow& window);
    void show(int screenId);

private:
    Screen* screens[ScreenId::COUNT];
    int activeScreen;
    GameSession session;
};

}
