#include "gui/widgets/AlarmMeter.h"
#include <string>

using namespace std;

namespace vh {

void AlarmMeter::draw(sf::RenderTarget& target, Renderer& renderer, const ScoreSystem& score,
                      sf::Vector2f position, float width) const {
    const float height = 22.f;
    sf::RectangleShape background({width, height});
    background.setPosition(position);
    background.setFillColor(Palette::PANEL);
    background.setOutlineColor(Palette::BORDER);
    background.setOutlineThickness(2.f);
    target.draw(background);

    float ratio = static_cast<float>(score.alarm()) / static_cast<float>(score.alarmCapacity());
    sf::RectangleShape fill({width * ratio, height});
    fill.setPosition(position);
    if (ratio < 0.5f) fill.setFillColor(Palette::BALANCED);
    else if (ratio < 0.8f) fill.setFillColor(Palette::WARNING);
    else fill.setFillColor(Palette::VIOLATION);
    target.draw(fill);

    string label = "ALARM " + to_string(score.alarm()) + "/" + to_string(score.alarmCapacity());
    renderer.drawText(label, {position.x + width / 2.f, position.y + height / 2.f}, 13, Palette::TEXT, true);
}

}
