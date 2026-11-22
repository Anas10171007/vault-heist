#pragma once
#include "gui/screens/StubScreen.h"
#include "gui/core/AnimationController.h"

namespace vh {

class AnimationDemoScreen : public StubScreen, public AnimationListener {
public:
    AnimationDemoScreen(Theme& theme);
    int handleEvent(const sf::Event& event, sf::RenderWindow& window) override;
    int update(float seconds) override;
    void draw(sf::RenderWindow& window) override;
    void reset() override;
    void onEvent(const Event& event, float progress) override;
private:
    Theme& demoTheme;
    AnimationController animation;
    float progress[5];
};

}
