#pragma once
#include "game/GameSession.h"
#include "game/LevelManager.h"
#include "game/LevelPlay.h"
#include "gui/core/Screen.h"
#include "gui/core/Theme.h"
#include "gui/screens/HeapLevelScreen.h"
#include "gui/widgets/AlarmMeter.h"

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
    LevelPlay* logic;
    AlarmMeter alarm;
    bool heapMode;
    string message;
    sf::FloatRect nextButton() const;
    sf::FloatRect hintButton() const;
    sf::FloatRect resetButton() const;
    sf::FloatRect exitButton() const;
    void applyNext();
    void storeResult(const ScoreSystem& score);
};
}
