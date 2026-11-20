#include "gui/core/Renderer.h"
#include <fstream>
bool loadDefaultFont(sf::Font& font) {
    const char* prefixes[4] = {"", "../", "../../", "../../../"};
    for (int i = 0; i < 4; i++) {
        std::string path = std::string(prefixes[i]) + "assets/fonts/Inter-Regular.ttf";
        std::ifstream probe(path, std::ios::binary);
        if (probe.good() && font.openFromFile(path)) return true;
    }
    return false;
}
Renderer::Renderer(sf::RenderTarget& t, const sf::Font& font) : target(t), text(font, "", 20), lineShape(), hexShape(NODE_RADIUS, 6) {}
void Renderer::drawText(const std::string& utf8, sf::Vector2f pos, unsigned int size, sf::Color color, bool centered) {
    text.setString(sf::String::fromUtf8(utf8.begin(), utf8.end()));
    text.setCharacterSize(size);
    text.setFillColor(color);
    sf::FloatRect b = text.getLocalBounds();
    if (centered) text.setOrigin(b.position + b.size / 2.f);
    else text.setOrigin({0.f, 0.f});
    text.setPosition(pos);
    target.draw(text);
}
void Renderer::drawLine(sf::Vector2f a, sf::Vector2f b, float thickness, sf::Color color) {
    sf::Vector2f d = b - a;
    float len = d.length();
    if (len < 0.001f) return;
    lineShape.setSize({len, thickness});
    lineShape.setOrigin({0.f, thickness / 2.f});
    lineShape.setPosition(a);
    lineShape.setRotation(d.angle());
    lineShape.setFillColor(color);
    target.draw(lineShape);
}
void Renderer::drawEdge(sf::Vector2f a, sf::Vector2f b, bool highlight) {
    if (highlight) drawLine(a, b, EDGE_THICKNESS_ACTIVE, Palette::VISITED);
    else drawLine(a, b, EDGE_THICKNESS, Palette::EDGE);
}
void Renderer::drawNode(sf::Vector2f pos, const std::string& label, int state) {
    if (state < 0 || state >= NodeState::COUNT) state = NodeState::NORMAL;
    hexShape.setPosition(pos);
    if (state == NodeState::TARGET) {
        float ring = NODE_RADIUS + 7.f;
        hexShape.setRadius(ring);
        hexShape.setOrigin({ring, ring});
        hexShape.setFillColor(sf::Color::Transparent);
        hexShape.setOutlineColor(Palette::TARGET);
        hexShape.setOutlineThickness(2.f);
        target.draw(hexShape);
    }
    hexShape.setRadius(NODE_RADIUS);
    hexShape.setOrigin({NODE_RADIUS, NODE_RADIUS});
    hexShape.setFillColor(Palette::stateColor(state));
    if (state == NodeState::VIOLATION) {
        hexShape.setOutlineColor(sf::Color::White);
        hexShape.setOutlineThickness(4.f);
    } else {
        hexShape.setOutlineColor(state == NodeState::NORMAL ? Palette::NODE_STROKE : Palette::INK);
        hexShape.setOutlineThickness(2.5f);
    }
    target.draw(hexShape);
        if (state == NodeState::SKIPPED) {
        float k = NODE_RADIUS * 0.55f;
        drawLine(pos + sf::Vector2f(-k, -k), pos + sf::Vector2f(k, k), 2.f, Palette::BORDER);
        drawLine(pos + sf::Vector2f(-k, k), pos + sf::Vector2f(k, -k), 2.f, Palette::BORDER);
    }
    drawText(label, pos, 17, state == NodeState::NORMAL ? Palette::TEXT : Palette::INK, true);
    const char* tag = NodeState::tag(state);
        if (tag[0] != '\0') drawChip({pos.x, pos.y + NODE_RADIUS + 14.f}, tag, 12, Palette::CHIP, Palette::TEXT);
}
void Renderer::drawChip(sf::Vector2f pos, const std::string& s, unsigned int size, sf::Color fill, sf::Color ink) {
    text.setString(sf::String::fromUtf8(s.begin(), s.end()));
    text.setCharacterSize(size);
    float w = text.getLocalBounds().size.x + 12.f;
    float h = static_cast<float>(size) + 7.f;
    lineShape.setSize({w, h});
    lineShape.setOrigin({w / 2.f, h / 2.f});
    lineShape.setPosition(pos);
    lineShape.setRotation(sf::degrees(0.f));
    lineShape.setFillColor(fill);
    target.draw(lineShape);
    drawText(s, pos, size, ink, true);
}
void Renderer::drawBadge(sf::Vector2f pos, const std::string& badgeText, int state) {
    if (state < 0 || state >= NodeState::COUNT) state = NodeState::NORMAL;
    drawChip(pos, badgeText, 13, Palette::stateColor(state), Palette::INK);
}
void Renderer::drawArrayCell(sf::Vector2f pos, int index, int value, int region) {
    if (region < 0 || region >= CellRegion::COUNT) region = CellRegion::ACTIVE;
    sf::Color fill = Palette::NODE_FILL;
    sf::Color edge = Palette::NODE_STROKE;
    sf::Color ink = Palette::TEXT;
    float thickness = 2.5f;
    if (region == CellRegion::SORTED) {
        fill = Palette::PANEL;
        edge = Palette::MUTED;
        ink = Palette::MUTED;
    } else if (region == CellRegion::SWAPPING) {
        fill = Palette::VISITED;
        edge = Palette::INK;
        ink = Palette::INK;
        thickness = 4.f;
    } else if (region == CellRegion::SELECTED) {
        fill = Palette::TARGET;
        edge = sf::Color::White;
        ink = Palette::INK;
        thickness = 4.f;
    }
    cellShape.setSize({CELL_W, CELL_H});
    cellShape.setOrigin({CELL_W / 2.f, CELL_H / 2.f});
    cellShape.setPosition(pos);
    cellShape.setFillColor(fill);
    cellShape.setOutlineColor(edge);
    cellShape.setOutlineThickness(thickness);
    target.draw(cellShape);
    drawText(std::to_string(value), pos, 18, ink, true);
    drawText(std::to_string(index), {pos.x, pos.y + CELL_H / 2.f + 12.f}, 11, Palette::MUTED, true);
    if (region == CellRegion::SORTED) drawText("\u2713", {pos.x + CELL_W / 2.f - 9.f, pos.y - CELL_H / 2.f + 9.f}, 13, Palette::BALANCED, true);
    if (region == CellRegion::SWAPPING) drawText("\u2194", {pos.x + CELL_W / 2.f - 10.f, pos.y - CELL_H / 2.f + 9.f}, 13, Palette::INK, true);
}
