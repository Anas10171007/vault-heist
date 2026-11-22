#pragma once
#include "game/GameSession.h"
#include "game/LevelManager.h"
#include "gui/core/Screen.h"
#include "gui/core/Theme.h"
#include "gui/screens/HeapLevelScreen.h"
#include "gui/screens/TreeMissionScreen.h"

namespace vh {
class LevelScreen : public Screen {
public:
    LevelScreen(Theme& theme, GameSession& session);
    ~LevelScreen() override;
    int handleEvent(const sf::Event& event, sf::RenderWindow& window) override;
    int update(float seconds) override;
    void draw(sf::RenderWindow& window) override;
    void onShow() override;
private:
    Theme& theme;
    GameSession& session;
    LevelManager levels;
    HeapLevelScreen heapScreen;
    TreeMissionScreen mission;
    bool heapMode;
    bool treeMode;
    string message;

    void storeResult(const ScoreSystem& score);
    void prepareTreeLevel(const LevelDef& level);
};
}
