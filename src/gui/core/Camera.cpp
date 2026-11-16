#include "gui/core/Camera.h"
Camera::Camera(sf::Vector2f size) : view(size / 2.f, size), windowSize(size), scale(1.f), dragging(false), lastPixel(0, 0) {}
float Camera::clampScale(float s) {
    if (s < MIN_SCALE) return MIN_SCALE;
    if (s > MAX_SCALE) return MAX_SCALE;
    return s;
}
void Camera::refreshSize() {
    view.setSize(windowSize * scale);
}
sf::Vector2f Camera::toWorld(sf::Vector2i pixel) const {
    sf::Vector2f offset(static_cast<float>(pixel.x) - windowSize.x / 2.f, static_cast<float>(pixel.y) - windowSize.y / 2.f);
    return view.getCenter() + offset * scale;
}
float Camera::zoomFactor() const {
    return 1.f / scale;
}
void Camera::apply(sf::RenderTarget& target) const {
    target.setView(view);
}
void Camera::fitToView(sf::FloatRect bounds, float margin) {
    float needX = (bounds.size.x + 2.f * margin) / windowSize.x;
    float needY = (bounds.size.y + 2.f * margin) / windowSize.y;
    scale = clampScale(needX > needY ? needX : needY);
    view.setCenter(bounds.position + bounds.size / 2.f);
    refreshSize();
}
void Camera::handleEvent(const sf::Event& event) {
    if (const auto* wheel = event.getIf<sf::Event::MouseWheelScrolled>()) {
        if (wheel->delta == 0.f) return;
        sf::Vector2f before = toWorld(wheel->position);
        scale = clampScale(wheel->delta > 0.f ? scale * 0.9f : scale * 1.1f);
        refreshSize();
        sf::Vector2f after = toWorld(wheel->position);
        view.move(before - after);
    } else if (const auto* press = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (press->button == sf::Mouse::Button::Left) {
            dragging = true;
            lastPixel = press->position;
        }
    } else if (const auto* release = event.getIf<sf::Event::MouseButtonReleased>()) {
        if (release->button == sf::Mouse::Button::Left) dragging = false;
    } else if (const auto* moved = event.getIf<sf::Event::MouseMoved>()) {
        if (dragging) {
            sf::Vector2i d = lastPixel - moved->position;
            view.move(sf::Vector2f(static_cast<float>(d.x), static_cast<float>(d.y)) * scale);
            lastPixel = moved->position;
        }
    } else if (const auto* resized = event.getIf<sf::Event::Resized>()) {
        if (resized->size.x == 0 || resized->size.y == 0) return;
        windowSize = sf::Vector2f(static_cast<float>(resized->size.x), static_cast<float>(resized->size.y));
        refreshSize();
    }
}