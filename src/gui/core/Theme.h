#pragma once
#include <SFML/Graphics.hpp>
#include <string>

using namespace std;

namespace vh {

class Theme {
public:
    Theme();
    bool loadFont(const string& path);
    sf::Text makeLabel(const string& text, unsigned int size, sf::Color color) const;
    sf::RectangleShape makePanel(sf::Vector2f position, sf::Vector2f size) const;
    sf::RectangleShape makeButton(sf::Vector2f position, sf::Vector2f size) const;

    sf::Color background;
    sf::Color panel;
    sf::Color text;
    sf::Color green;
    sf::Color yellow;
    sf::Color red;
    sf::Color blue;
    sf::Color grey;
    sf::Color gold;

private:
    sf::Font font;
};

}
