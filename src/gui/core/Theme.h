#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include "gui/core/Style.h"

using namespace std;

namespace vh {

class Theme {
public:
    Theme();
    bool loadFont();
    sf::Text makeLabel(const string& text, unsigned int size, sf::Color color) const;
    sf::RectangleShape makePanel(sf::Vector2f position, sf::Vector2f size) const;
    sf::RectangleShape makeButton(sf::Vector2f position, sf::Vector2f size) const;

private:
    sf::Font font;
};

}
