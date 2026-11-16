#include "gui/core/Theme.h"

namespace vh {

Theme::Theme() {
    background = sf::Color(18, 20, 32);
    panel = sf::Color(35, 39, 58);
    text = sf::Color(238, 238, 242);

    green = sf::Color(64, 170, 105);
    yellow = sf::Color(230, 190, 70);
    red = sf::Color(205, 75, 75);
    blue = sf::Color(75, 140, 220);
    grey = sf::Color(125, 130, 145);
    gold = sf::Color(220, 170, 55);
}

bool Theme::loadFont(const string& path) {
    return font.openFromFile(path);
}

sf::Text Theme::makeLabel(const string& label, unsigned int size, sf::Color color) const {
    sf::Text result(font, label, size);
    result.setFillColor(color);
    return result;
}

sf::RectangleShape Theme::makePanel(sf::Vector2f position, sf::Vector2f size) const {
    sf::RectangleShape result(size);
    result.setPosition(position);
    result.setFillColor(panel);
    result.setOutlineColor(grey);
    result.setOutlineThickness(2.f);
    return result;
}

sf::RectangleShape Theme::makeButton(sf::Vector2f position, sf::Vector2f size) const {
    sf::RectangleShape result(size);
    result.setPosition(position);
    result.setFillColor(blue);
    result.setOutlineColor(text);
    result.setOutlineThickness(2.f);
    return result;
}

}
