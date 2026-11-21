#include "gui/widgets/AlarmMeter.h"
#include <cmath>
#include <cstdint>
#include <string>

using namespace std;

namespace vh {

void AlarmMeter::draw(sf::RenderTarget& target, Renderer& renderer, const ScoreSystem& score,
                      sf::Vector2f position, float width, float timeSeconds) const {
    const float height = 18.f;
    float ratio = 0.f;
    if (score.alarmCapacity() > 0) {
        ratio = static_cast<float>(score.alarm()) / static_cast<float>(score.alarmCapacity());
    }
    if (ratio > 1.f) ratio = 1.f;

    renderer.drawText("ALARM", {position.x, position.y - 16.f}, 11, Palette::MUTED, false);

    sf::RectangleShape track({width, height});
    track.setPosition(position);
    track.setFillColor(Palette::BG);
    track.setOutlineColor(Palette::BORDER);
    track.setOutlineThickness(1.5f);
    target.draw(track);

    const int segments = 12;
    float gap = 2.f;
    float segW = (width - gap * (segments - 1)) / segments;
    int lit = static_cast<int>(ratio * segments + 0.001f);
    sf::Color fill = Palette::BALANCED;
    if (ratio >= 0.8f) fill = Palette::VIOLATION;
    else if (ratio >= 0.5f) fill = Palette::WARNING;

    if (ratio >= 0.85f) {
        float pulse = 0.55f + 0.45f * (0.5f + 0.5f * sin(timeSeconds * 8.f));
        fill = sf::Color(static_cast<uint8_t>(220 * pulse + 35),
                         static_cast<uint8_t>(64 * pulse),
                         static_cast<uint8_t>(72 * pulse));
    }

    for (int i = 0; i < segments; i++) {
        sf::RectangleShape seg({segW, height - 4.f});
        seg.setPosition({position.x + i * (segW + gap), position.y + 2.f});
        seg.setFillColor(i < lit ? fill : Palette::PANEL_HI);
        target.draw(seg);
    }

    string label = to_string(score.alarm()) + " / " + to_string(score.alarmCapacity());
    renderer.drawText(label, {position.x + width, position.y - 16.f}, 11, Palette::TEXT, false);
}

}
