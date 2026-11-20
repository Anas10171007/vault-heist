#pragma once
#include <string>
#include "game/LevelDef.h"
#include "game/act4/HeapBuildLevel.h"
#include "gui/core/AnimationController.h"
#include "gui/core/Screen.h"
#include "gui/core/Theme.h"
#include "gui/widgets/AlarmMeter.h"
#include "gui/widgets/HeapView.h"

using namespace std;

namespace vh {

class HeapLevelScreen : public Screen {
public:
    HeapLevelScreen(Theme& theme);
    int handleEvent(const sf::Event& event, sf::RenderWindow& window) override;
    void update(float seconds) override;
    void draw(sf::RenderWindow& window) override;
    void onShow() override;

private:
    Theme& theme;
    LevelDef level;
    bool levelLoaded;
    HeapBuildLevel logic;
    HeapView heapView;
    AnimationController animation;
    AlarmMeter alarmMeter;
    int firstSelection;
    bool waitingForAnimation;
    string message;

    void resetLevel();
    void selectIndex(int index);
    bool loadLevel();
    void drawButton(sf::RenderWindow& window, Renderer& renderer,
                    sf::FloatRect bounds, const string& label) const;
    sf::FloatRect resetButton() const;
    sf::FloatRect continueButton() const;
    sf::FloatRect menuButton() const;
};

}
