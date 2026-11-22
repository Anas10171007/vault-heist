#pragma once
#include <string>
#include "gui/core/Screen.h"
#include "gui/core/ScreenId.h"
#include "gui/core/Theme.h"

using namespace std;

namespace vh {

class StubScreen : public Screen {
public:
    StubScreen(Theme& theme, const string& title, const string& description);
    void addButton(const string& label, int targetScreen);
    int handleEvent(const sf::Event& event, sf::RenderWindow& window) override;
    int update(float deltaSeconds) override;
    void draw(sf::RenderWindow& window) override;

private:
    struct NavigationButton {
        string label;
        int targetScreen;
    };

    static const int MAX_BUTTONS = 6;
    Theme& theme;
    string title;
    string description;
    NavigationButton buttons[MAX_BUTTONS];
    int buttonCount;

    sf::FloatRect buttonBounds(int index) const;
};

}
