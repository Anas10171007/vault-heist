#pragma once
#include "game/GameSession.h"
#include "game/LevelManager.h"
#include "game/LevelPlay.h"
#include "gui/core/Screen.h"
#include "gui/core/Theme.h"
#include "gui/screens/HeapLevelScreen.h"
#include "gui/widgets/LevelChrome.h"
#include "model/EventLog.h"

namespace vh {
class LevelScreen : public Screen {
public:
    LevelScreen(Theme& theme, GameSession& session);
    ~LevelScreen() override;
    int handleEvent(const sf::Event& event, sf::RenderWindow& window) override;
    void update(float seconds) override;
    void draw(sf::RenderWindow& window) override;
    void onShow() override;
private:
    Theme& theme;
    GameSession& session;
    LevelManager levels;
    HeapLevelScreen heapScreen;
    LevelChrome chrome;
    LevelPlay* logic;
    EventLog lastEvents;
    bool heapMode;
    string message;
    float viewZoom;
    float viewX;
    float viewY;
    bool dragging;
    sf::Vector2i lastDragPixel;
    void applyNext();
    void undo();
    void showHint();
    void storeResult(const ScoreSystem& score);
};
}
