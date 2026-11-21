#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include "gui/core/AnimationController.h"
#include "gui/core/Renderer.h"
#include "gui/core/Theme.h"

using namespace std;

namespace vh {

class ReplayScrubber {
public:
    static sf::FloatRect trackBounds() { return {{48.f, 640.f}, {700.f, 14.f}}; }

    void draw(sf::RenderWindow& window, Renderer& renderer, Theme& theme,
              const AnimationController& animation) const {
        sf::FloatRect track = trackBounds();
        sf::RectangleShape back(track.size);
        back.setPosition(track.position);
        back.setFillColor(Palette::BG);
        back.setOutlineColor(Palette::BORDER);
        back.setOutlineThickness(1.5f);
        window.draw(back);

        int total = animation.eventCount();
        float ratio = 0.f;
        if (total > 0) {
            ratio = static_cast<float>(animation.completedCount()) / static_cast<float>(total);
        }
        if (ratio > 1.f) ratio = 1.f;

        sf::RectangleShape fill({track.size.x * ratio, track.size.y});
        fill.setPosition(track.position);
        fill.setFillColor(Palette::VISITED);
        window.draw(fill);

        float knobX = track.position.x + track.size.x * ratio;
        sf::CircleShape knob(7.f);
        knob.setOrigin({7.f, 7.f});
        knob.setPosition({knobX, track.position.y + track.size.y / 2.f});
        knob.setFillColor(Palette::BRASS);
        window.draw(knob);

        string label = "REPLAY  " + to_string(animation.completedCount()) + " / " +
                       to_string(total);
        renderer.drawText(label, {track.position.x, track.position.y - 18.f}, 11,
                          Palette::MUTED, false);
        (void)theme;
    }

    // Returns true if the scrubber handled the click (and sought).
    bool handleClick(const sf::RenderWindow& /*window*/, sf::Vector2f point,
                     AnimationController& animation) {
        sf::FloatRect track = trackBounds();
        sf::FloatRect hit = track;
        hit.position.y -= 8.f;
        hit.size.y += 16.f;
        if (!hit.contains(point) || animation.eventCount() <= 0) return false;

        float t = (point.x - track.position.x) / track.size.x;
        if (t < 0.f) t = 0.f;
        if (t > 1.f) t = 1.f;
        int index = static_cast<int>(t * animation.eventCount() + 0.5f);
        animation.seekTo(index);
        dragging = true;
        return true;
    }

    bool handleDrag(const sf::RenderWindow& window, AnimationController& animation) {
        if (!dragging || animation.eventCount() <= 0) return false;
        if (!sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
            dragging = false;
            return false;
        }
        sf::Vector2f point = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        sf::FloatRect track = trackBounds();
        float t = (point.x - track.position.x) / track.size.x;
        if (t < 0.f) t = 0.f;
        if (t > 1.f) t = 1.f;
        int index = static_cast<int>(t * animation.eventCount() + 0.5f);
        animation.seekTo(index);
        return true;
    }

    void release() { dragging = false; }
    bool isDragging() const { return dragging; }

private:
    bool dragging = false;
};

}
