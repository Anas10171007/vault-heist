#pragma once
#include <SFML/Graphics.hpp>
#include "game/ScoreSystem.h"
#include "gui/core/Renderer.h"

namespace vh {

class AlarmMeter {
public:
    void draw(sf::RenderTarget& target, Renderer& renderer, const ScoreSystem& score,
              sf::Vector2f position, float width) const;
};

}
