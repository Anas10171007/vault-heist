#pragma once
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/Vector2.hpp>
#include <string>
#include "gui/core/Style.h"
bool loadDefaultFont(sf::Font& font);
class Renderer {
public:
    static constexpr float NODE_RADIUS = 24.f;
    static constexpr float NODE_HIT_RADIUS = 20.78f;
        static constexpr float CELL_W = 52.f;
    static constexpr float CELL_H = 44.f;
    Renderer(sf::RenderTarget& target, const sf::Font& font);
    void drawText(const std::string& utf8, sf::Vector2f pos, unsigned int size, sf::Color color, bool centered);
    void drawEdge(sf::Vector2f a, sf::Vector2f b, bool highlight);
        void drawNode(sf::Vector2f pos, const std::string& label, int state);
    void drawBadge(sf::Vector2f pos, const std::string& text, int state);
        void drawArrayCell(sf::Vector2f pos, int index, int value, int region);
private:
    static constexpr float EDGE_THICKNESS = 3.f;
    static constexpr float EDGE_THICKNESS_ACTIVE = 6.f;
    void drawLine(sf::Vector2f a, sf::Vector2f b, float thickness, sf::Color color);
        void drawChip(sf::Vector2f pos, const std::string& s, unsigned int size, sf::Color fill, sf::Color ink);
    sf::RenderTarget& target;
    sf::Text text;
    sf::RectangleShape lineShape;
    sf::CircleShape hexShape;
    sf::RectangleShape cellShape;
};