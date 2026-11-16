#pragma once
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Event.hpp>
class Camera {
public:
    explicit Camera(sf::Vector2f size);
    void handleEvent(const sf::Event& event);
    void apply(sf::RenderTarget& target) const;
    void fitToView(sf::FloatRect bounds, float margin);
    sf::Vector2f toWorld(sf::Vector2i pixel) const;
    float zoomFactor() const;
private:
    static constexpr float MIN_SCALE = 0.05f;
    static constexpr float MAX_SCALE = 20.f;
    static float clampScale(float s);
    void refreshSize();
    sf::View view;
    sf::Vector2f windowSize;
    float scale;
    bool dragging;
    sf::Vector2i lastPixel;
};