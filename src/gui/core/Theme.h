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
    const sf::Font& getFont() const;
    float timeSeconds() const;

    sf::Text makeLabel(const string& text, unsigned int size, sf::Color color) const;
    sf::RectangleShape makePanel(sf::Vector2f position, sf::Vector2f size) const;
    sf::RectangleShape makeButton(sf::Vector2f position, sf::Vector2f size) const;
    sf::RectangleShape makeButton(sf::Vector2f position, sf::Vector2f size, int kind,
                                  bool hovered) const;

    void drawBackdrop(sf::RenderWindow& window) const;
    void drawPanel(sf::RenderWindow& window, sf::FloatRect bounds) const;
    void drawButton(sf::RenderWindow& window, const sf::Font& font, sf::FloatRect bounds,
                    const string& label, int kind, bool hovered) const;
    bool hovered(const sf::RenderWindow& window, sf::FloatRect bounds) const;

private:
    sf::Font font;
    sf::Clock clock;
    bool tryLoad(const string& path);
};

}
