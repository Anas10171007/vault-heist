#include "gui/core/Theme.h"
#include <fstream>

namespace vh {

Theme::Theme() = default;

bool Theme::loadFont() {
    const char* prefixes[4] = {"", "../", "../../", "../../../"};
    for (int i = 0; i < 4; i++) {
        string path = string(prefixes[i]) + "assets/fonts/Inter-Regular.ttf";
        ifstream file(path, ios::binary);
        if (file.good() && font.openFromFile(path)) return true;
    }
    return false;
}

sf::Text Theme::makeLabel(const string& label, unsigned int size, sf::Color color) const {
    sf::Text result(font, sf::String::fromUtf8(label.begin(), label.end()), size);
    result.setFillColor(color);
    return result;
}

sf::RectangleShape Theme::makePanel(sf::Vector2f position, sf::Vector2f size) const {
    sf::RectangleShape result(size);
    result.setPosition(position);
    result.setFillColor(Palette::PANEL);
    result.setOutlineColor(Palette::BORDER);
    result.setOutlineThickness(2.f);
    return result;
}

sf::RectangleShape Theme::makeButton(sf::Vector2f position, sf::Vector2f size) const {
    sf::RectangleShape result(size);
    result.setPosition(position);
    result.setFillColor(Palette::VISITED);
    result.setOutlineColor(Palette::TEXT);
    result.setOutlineThickness(2.f);
    return result;
}

}
