#include "gui/core/Theme.h"
#include <cmath>
#include <fstream>

namespace vh {

Theme::Theme() = default;

bool Theme::tryLoad(const string& path) {
    ifstream file(path, ios::binary);
    return file.good() && font.openFromFile(path);
}

bool Theme::loadFont() {
    const char* prefixes[4] = {"", "../", "../../", "../../../"};
    const char* names[3] = {
        "assets/fonts/Bahnschrift.ttf",
        "assets/fonts/CascadiaCode.ttf",
        "assets/fonts/Inter-Regular.ttf",
    };
    for (int p = 0; p < 4; p++) {
        for (int n = 0; n < 3; n++) {
            if (tryLoad(string(prefixes[p]) + names[n])) return true;
        }
    }
    return false;
}

const sf::Font& Theme::getFont() const { return font; }

float Theme::timeSeconds() const {
    return clock.getElapsedTime().asSeconds();
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
    result.setOutlineThickness(1.5f);
    return result;
}

sf::RectangleShape Theme::makeButton(sf::Vector2f position, sf::Vector2f size) const {
    return makeButton(position, size, ButtonKind::PRIMARY, false);
}

sf::RectangleShape Theme::makeButton(sf::Vector2f position, sf::Vector2f size, int kind,
                                     bool hovered) const {
    sf::RectangleShape result(size);
    result.setPosition(position);
    sf::Color fill = Palette::PANEL_HI;
    sf::Color edge = Palette::BORDER_HI;
    if (kind == ButtonKind::PRIMARY) {
        fill = hovered ? sf::Color(84, 198, 188) : Palette::VISITED;
        edge = Palette::BRASS;
    } else if (kind == ButtonKind::SECONDARY) {
        fill = hovered ? Palette::PANEL_HI : Palette::PANEL;
        edge = hovered ? Palette::BRASS : Palette::BORDER_HI;
    } else if (kind == ButtonKind::GHOST) {
        fill = sf::Color(255, 255, 255, hovered ? 18 : 8);
        edge = Palette::BORDER;
    } else if (kind == ButtonKind::DANGER) {
        fill = hovered ? sf::Color(240, 84, 92) : Palette::VIOLATION;
        edge = Palette::TEXT;
    }
    result.setFillColor(fill);
    result.setOutlineColor(edge);
    result.setOutlineThickness(hovered ? 2.2f : 1.5f);
    return result;
}

bool Theme::hovered(const sf::RenderWindow& window, sf::FloatRect bounds) const {
    sf::Vector2f point = window.mapPixelToCoords(sf::Mouse::getPosition(window));
    return bounds.contains(point);
}

void Theme::drawBackdrop(sf::RenderWindow& window) const {
    sf::Vector2u size = window.getSize();
    float w = static_cast<float>(size.x);
    float h = static_cast<float>(size.y);

    sf::VertexArray wash(sf::PrimitiveType::TriangleStrip, 4);
    wash[0].position = {0.f, 0.f};
    wash[1].position = {w, 0.f};
    wash[2].position = {0.f, h};
    wash[3].position = {w, h};
    wash[0].color = Palette::BG_TOP;
    wash[1].color = Palette::BG_TOP;
    wash[2].color = Palette::BG;
    wash[3].color = Palette::BG;
    window.draw(wash);

    float t = timeSeconds();
    float scanY = fmod(t * 38.f, h + 80.f) - 40.f;
    sf::RectangleShape scan({w, 48.f});
    scan.setPosition({0.f, scanY});
    scan.setFillColor(Palette::SCAN);
    window.draw(scan);

    sf::RectangleShape grid;
    grid.setFillColor(sf::Color(255, 255, 255, 7));
    for (float x = 40.f; x < w; x += 40.f) {
        grid.setSize({1.f, h});
        grid.setPosition({x, 0.f});
        window.draw(grid);
    }
    for (float y = 40.f; y < h; y += 40.f) {
        grid.setSize({w, 1.f});
        grid.setPosition({0.f, y});
        window.draw(grid);
    }

    sf::RectangleShape vignette({w, 90.f});
    vignette.setFillColor(sf::Color(0, 0, 0, 90));
    vignette.setPosition({0.f, h - 90.f});
    window.draw(vignette);
}

void Theme::drawPanel(sf::RenderWindow& window, sf::FloatRect bounds) const {
    sf::RectangleShape shadow(bounds.size + sf::Vector2f(6.f, 8.f));
    shadow.setPosition(bounds.position + sf::Vector2f(4.f, 6.f));
    shadow.setFillColor(sf::Color(0, 0, 0, 90));
    window.draw(shadow);

    sf::RectangleShape panel = makePanel(bounds.position, bounds.size);
    window.draw(panel);

    sf::RectangleShape accent({4.f, bounds.size.y});
    accent.setPosition(bounds.position);
    accent.setFillColor(Palette::BRASS);
    window.draw(accent);
}

void Theme::drawButton(sf::RenderWindow& window, const sf::Font& labelFont, sf::FloatRect bounds,
                       const string& label, int kind, bool isHovered) const {
    window.draw(makeButton(bounds.position, bounds.size, kind, isHovered));
    sf::Text text(labelFont, sf::String::fromUtf8(label.begin(), label.end()), 16);
    text.setFillColor(kind == ButtonKind::PRIMARY ? Palette::INK : Palette::TEXT);
    if (kind == ButtonKind::DANGER) text.setFillColor(Palette::TEXT);
    sf::FloatRect box = text.getLocalBounds();
    text.setOrigin({box.position.x + box.size.x / 2.f, box.position.y + box.size.y / 2.f});
    text.setPosition(bounds.position + bounds.size / 2.f);
    window.draw(text);
}

}
