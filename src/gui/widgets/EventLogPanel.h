#pragma once
#include <SFML/Graphics.hpp>
#include "gui/core/Renderer.h"
#include "gui/widgets/EventDescribe.h"
#include "model/EventLog.h"

namespace vh {

class EventLogPanel {
public:
    void draw(Renderer& renderer, const EventLog& log, int highlightIndex,
              sf::Vector2f position, float /*width*/, int maxLines = 6) const {
        renderer.drawText("EVENT LOG", position, 14, Palette::MUTED, false);
        if (log.empty()) {
            renderer.drawText("(no events yet)",
                              position + sf::Vector2f(0.f, 22.f), 12, Palette::MUTED, false);
            return;
        }

        int start = log.size() - maxLines;
        if (start < 0) start = 0;
        float y = position.y + 22.f;
        for (int index = start; index < log.size(); index++) {
            string line = to_string(index + 1) + ". " + describeEvent(log.at(index));
            sf::Color color = Palette::TEXT;
            if (index == highlightIndex) color = Palette::TARGET;
            renderer.drawText(line, {position.x, y}, 11, color, false);
            y += 16.f;
        }
    }
};

}
