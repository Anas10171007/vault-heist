#pragma once
#include <SFML/Graphics.hpp>
#include "gui/core/Renderer.h"
#include "gui/widgets/EventDescribe.h"
#include "model/EventLog.h"

namespace vh {

class EventLogPanel {
public:
    void draw(Renderer& renderer, const EventLog& log, int highlightIndex,
              sf::Vector2f position, float /*width*/, int maxLines = 5) const {
        renderer.drawText("EVENT LOG", position, 11, Palette::MUTED, false);
        if (log.empty()) {
            renderer.drawText("Waiting for model events…",
                              position + sf::Vector2f(0.f, 20.f), 12, Palette::MUTED, false);
            return;
        }

        int start = log.size() - maxLines;
        if (start < 0) start = 0;
        float y = position.y + 20.f;
        for (int index = start; index < log.size(); index++) {
            string line = describeEvent(log.at(index));
            if (line.size() > 28) line = line.substr(0, 27) + "…";
            sf::Color color = index == highlightIndex ? Palette::BRASS : Palette::TEXT;
            renderer.drawText(line, {position.x, y}, 11, color, false);
            y += 15.f;
        }
    }
};

}
