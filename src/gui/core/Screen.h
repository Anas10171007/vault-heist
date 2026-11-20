#pragma once
#include <SFML/Graphics.hpp>

namespace vh {

class Screen {
public:
    virtual ~Screen() {}
    virtual int handleEvent(const sf::Event& event, sf::RenderWindow& window) = 0;
    virtual void update(float deltaSeconds) = 0;
    virtual void draw(sf::RenderWindow& window) = 0;
    virtual void onShow() {}
};

}
