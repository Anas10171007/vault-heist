#include "gui/core/Theme.h"
#include <cmath>
#include <cstdint>
#include <fstream>

namespace {

sf::Color withAlpha(sf::Color color, int value) {
    if (value < 0) value = 0;
    if (value > 255) value = 255;
    return sf::Color(color.r, color.g, color.b, static_cast<std::uint8_t>(value));
}

sf::Color shade(sf::Color color, float factor) {
    float r = static_cast<float>(color.r) * factor;
    float g = static_cast<float>(color.g) * factor;
    float b = static_cast<float>(color.b) * factor;
    if (r > 255.f) r = 255.f;
    if (g > 255.f) g = 255.f;
    if (b > 255.f) b = 255.f;
    return sf::Color(static_cast<std::uint8_t>(r), static_cast<std::uint8_t>(g),
                     static_cast<std::uint8_t>(b), color.a);
}

} // namespace

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

void Theme::drawVaultBackdrop(sf::RenderWindow& window) const {
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

    // Slow drifting hex field. Outlines only, so it stays cheap and subtle.
    sf::CircleShape hex(1.f, 6);
    for (int i = 0; i < 16; i++) {
        float phase = static_cast<float>(i) * 0.73f;
        float x = 60.f + static_cast<float>(i) * 79.f + std::sin(t * 0.07f + phase) * 26.f;
        float y = 30.f + static_cast<float>(i) * 53.f + std::cos(t * 0.05f + phase * 1.4f) * 20.f;
        x = std::fmod(x, w + 180.f) - 90.f;
        y = std::fmod(y, h + 180.f) - 90.f;
        float radius = 24.f + static_cast<float>(i % 4) * 15.f;
        hex.setRadius(radius);
        hex.setOrigin({radius, radius});
        hex.setPosition({x, y});
        hex.setRotation(sf::degrees(t * 3.f + phase * 11.f));
        hex.setFillColor(sf::Color::Transparent);
        hex.setOutlineThickness(1.f);
        hex.setOutlineColor(i % 5 == 0 ? sf::Color(212, 170, 84, 20)
                                       : sf::Color(64, 178, 168, 18));
        window.draw(hex);
    }

    sf::RectangleShape grid;
    grid.setFillColor(sf::Color(255, 255, 255, 6));
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

    float scanY = std::fmod(t * 34.f, h + 90.f) - 45.f;
    sf::RectangleShape scan({w, 54.f});
    scan.setPosition({0.f, scanY});
    scan.setFillColor(sf::Color(64, 178, 168, 14));
    window.draw(scan);

    sf::RectangleShape topShade({w, 90.f});
    topShade.setPosition({0.f, 0.f});
    topShade.setFillColor(sf::Color(0, 0, 0, 55));
    window.draw(topShade);

    sf::RectangleShape bottomShade({w, 120.f});
    bottomShade.setPosition({0.f, h - 120.f});
    bottomShade.setFillColor(sf::Color(0, 0, 0, 105));
    window.draw(bottomShade);
}

void Theme::drawGlowTitle(sf::RenderWindow& window, const string& text, sf::Vector2f position,
                          unsigned int size, sf::Color color, bool centered) const {
    sf::Text label(font, sf::String::fromUtf8(text.begin(), text.end()), size);
    sf::FloatRect box = label.getLocalBounds();
    if (centered) label.setOrigin(box.position + box.size / 2.f);
    else label.setOrigin({0.f, 0.f});

    // Same-size offsets keep the halo soft instead of doubling the glyphs.
    sf::Vector2f outer[8] = {{-3.f, 0.f}, {3.f, 0.f},  {0.f, -3.f}, {0.f, 3.f},
                             {-2.f, -2.f}, {2.f, -2.f}, {-2.f, 2.f}, {2.f, 2.f}};
    label.setFillColor(withAlpha(color, 8));
    for (int i = 0; i < 8; i++) {
        label.setPosition(position + outer[i]);
        window.draw(label);
    }

    sf::Vector2f inner[4] = {{-1.5f, 0.f}, {1.5f, 0.f}, {0.f, -1.5f}, {0.f, 1.5f}};
    label.setFillColor(withAlpha(color, 16));
    for (int i = 0; i < 4; i++) {
        label.setPosition(position + inner[i]);
        window.draw(label);
    }

    label.setFillColor(color);
    label.setPosition(position);
    window.draw(label);
}

void Theme::drawGlossyButton(sf::RenderWindow& window, const sf::Font& font, sf::FloatRect bounds,
                             const string& label, int kind, bool hovered) const {
    drawGlossyButton(window, font, bounds, label, "", kind, hovered);
}

void Theme::drawGlossyButton(sf::RenderWindow& window, const sf::Font& font, sf::FloatRect bounds,
                             const string& label, const string& sublabel, int kind,
                             bool hovered) const {
    bool pressed = hovered && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
    float drop = pressed ? 1.5f : 0.f;
    sf::FloatRect face = bounds;
    face.position.y += drop;

    sf::Color accent = Palette::BRASS;
    sf::Color top(34, 42, 58);
    sf::Color bottom(18, 23, 33);
    sf::Color border = hovered ? sf::Color(64, 200, 190) : Palette::BORDER_HI;
    sf::Color ink = Palette::TEXT;
    sf::Color subInk = Palette::MUTED;
    if (kind == ButtonKind::PRIMARY) {
        accent = Palette::BRASS;
        top = hovered ? sf::Color(100, 232, 218) : sf::Color(72, 210, 198);
        bottom = hovered ? sf::Color(40, 146, 140) : sf::Color(30, 118, 116);
        border = Palette::BRASS;
        ink = Palette::INK;
        subInk = withAlpha(Palette::INK, 190);
    } else if (kind == ButtonKind::GHOST) {
        accent = Palette::VISITED;
        top = sf::Color(26, 32, 44, 190);
        bottom = sf::Color(12, 16, 22, 190);
        border = hovered ? sf::Color(64, 200, 190) : Palette::BORDER;
        ink = hovered ? Palette::TEXT : Palette::MUTED;
        subInk = Palette::MUTED;
    } else if (kind == ButtonKind::DANGER) {
        accent = sf::Color(255, 170, 150);
        top = hovered ? sf::Color(240, 92, 98) : sf::Color(220, 76, 84);
        bottom = hovered ? sf::Color(160, 44, 52) : sf::Color(140, 36, 44);
        border = accent;
        ink = Palette::TEXT;
        subInk = withAlpha(Palette::TEXT, 200);
    }
    if (pressed) {
        top = shade(top, 0.86f);
        bottom = shade(bottom, 0.86f);
    }

    sf::RectangleShape shadow(bounds.size);
    shadow.setPosition(bounds.position + sf::Vector2f(3.f, 5.f));
    shadow.setFillColor(sf::Color(0, 0, 0, 115));
    window.draw(shadow);

    if (hovered) {
        for (int ring = 0; ring < 2; ring++) {
            float grow = 2.f + static_cast<float>(ring) * 3.f;
            sf::RectangleShape glow({face.size.x + grow * 2.f, face.size.y + grow * 2.f});
            glow.setPosition({face.position.x - grow, face.position.y - grow});
            glow.setFillColor(sf::Color::Transparent);
            glow.setOutlineColor(withAlpha(accent, ring == 0 ? 34 : 16));
            glow.setOutlineThickness(2.f);
            window.draw(glow);
        }
    }

    sf::VertexArray fill(sf::PrimitiveType::TriangleStrip, 4);
    fill[0].position = {face.position.x, face.position.y};
    fill[1].position = {face.position.x + face.size.x, face.position.y};
    fill[2].position = {face.position.x, face.position.y + face.size.y};
    fill[3].position = {face.position.x + face.size.x, face.position.y + face.size.y};
    fill[0].color = top;
    fill[1].color = top;
    fill[2].color = bottom;
    fill[3].color = bottom;
    window.draw(fill);

    sf::RectangleShape borderShape(face.size);
    borderShape.setPosition(face.position);
    borderShape.setFillColor(sf::Color::Transparent);
    borderShape.setOutlineColor(border);
    borderShape.setOutlineThickness(hovered ? 2.f : 1.f);
    window.draw(borderShape);

    sf::RectangleShape shine({face.size.x - 14.f, 1.5f});
    shine.setPosition({face.position.x + 7.f, face.position.y + 2.f});
    shine.setFillColor(sf::Color(255, 255, 255, pressed ? 26 : 48));
    window.draw(shine);

    sf::RectangleShape shortShine({face.size.x * 0.45f, 1.f});
    shortShine.setPosition({face.position.x + face.size.x * 0.275f, face.position.y + 4.5f});
    shortShine.setFillColor(sf::Color(255, 255, 255, pressed ? 12 : 22));
    window.draw(shortShine);

    sf::Text mainText(font, sf::String::fromUtf8(label.begin(), label.end()),
                      sublabel.empty() ? 16 : 15);
    mainText.setFillColor(ink);
    sf::FloatRect mainBox = mainText.getLocalBounds();
    mainText.setOrigin(mainBox.position + mainBox.size / 2.f);
    float mainY = sublabel.empty() ? face.position.y + face.size.y / 2.f
                                   : face.position.y + face.size.y * 0.34f;
    mainText.setPosition({face.position.x + face.size.x / 2.f, mainY});
    window.draw(mainText);

    if (!sublabel.empty()) {
        sf::Text subText(font, sf::String::fromUtf8(sublabel.begin(), sublabel.end()), 10);
        subText.setFillColor(subInk);
        sf::FloatRect subBox = subText.getLocalBounds();
        subText.setOrigin(subBox.position + subBox.size / 2.f);
        subText.setPosition({face.position.x + face.size.x / 2.f,
                             face.position.y + face.size.y * 0.72f});
        window.draw(subText);
    }
}

void Theme::drawCard(sf::RenderWindow& window, sf::FloatRect bounds, sf::Color accent,
                     bool hovered) const {
    sf::RectangleShape shadow(bounds.size);
    shadow.setPosition(bounds.position + sf::Vector2f(2.f, 4.f));
    shadow.setFillColor(sf::Color(0, 0, 0, 105));
    window.draw(shadow);

    sf::VertexArray fill(sf::PrimitiveType::TriangleStrip, 4);
    fill[0].position = {bounds.position.x, bounds.position.y};
    fill[1].position = {bounds.position.x + bounds.size.x, bounds.position.y};
    fill[2].position = {bounds.position.x, bounds.position.y + bounds.size.y};
    fill[3].position = {bounds.position.x + bounds.size.x, bounds.position.y + bounds.size.y};
    fill[0].color = hovered ? sf::Color(38, 47, 63) : sf::Color(29, 36, 49);
    fill[1].color = fill[0].color;
    fill[2].color = hovered ? sf::Color(19, 24, 34) : sf::Color(15, 19, 27);
    fill[3].color = fill[2].color;
    window.draw(fill);

    sf::RectangleShape border(bounds.size);
    border.setPosition(bounds.position);
    border.setFillColor(sf::Color::Transparent);
    border.setOutlineColor(hovered ? withAlpha(accent, 220) : Palette::BORDER);
    border.setOutlineThickness(hovered ? 1.6f : 1.f);
    window.draw(border);

    sf::RectangleShape bar({4.f, bounds.size.y});
    bar.setPosition(bounds.position);
    bar.setFillColor(accent);
    window.draw(bar);

    sf::RectangleShape shine({bounds.size.x - 10.f, 1.f});
    shine.setPosition({bounds.position.x + 7.f, bounds.position.y + 2.f});
    shine.setFillColor(sf::Color(255, 255, 255, hovered ? 30 : 16));
    window.draw(shine);
}

void Theme::drawStar(sf::RenderWindow& window, sf::Vector2f center, float radius,
                     bool filled) const {
    if (filled) {
        sf::CircleShape glow(radius * 1.8f, 12);
        glow.setOrigin({radius * 1.8f, radius * 1.8f});
        glow.setPosition(center);
        glow.setFillColor(sf::Color(212, 170, 84, 20));
        window.draw(glow);
    }

    sf::ConvexShape star;
    star.setPointCount(10);
    for (int i = 0; i < 10; i++) {
        float angle = -90.f + static_cast<float>(i) * 36.f;
        float r = (i % 2 == 0) ? radius : radius * 0.45f;
        float radians = angle * 3.14159265f / 180.f;
        star.setPoint(i, {std::cos(radians) * r, std::sin(radians) * r});
    }
    star.setPosition(center);
    star.setFillColor(filled ? Palette::BRASS : sf::Color::Transparent);
    star.setOutlineColor(filled ? sf::Color(255, 232, 176) : withAlpha(Palette::BRASS, 150));
    star.setOutlineThickness(filled ? 1.4f : 1.2f);
    window.draw(star);
}

void Theme::drawChip(sf::RenderWindow& window, sf::Vector2f center, const string& text,
                     unsigned int size, sf::Color accent) const {
    sf::Text label(font, sf::String::fromUtf8(text.begin(), text.end()), size);
    float width = label.getLocalBounds().size.x + 24.f;
    float height = static_cast<float>(size) + 13.f;

    sf::RectangleShape chip({width, height});
    chip.setOrigin({width / 2.f, height / 2.f});
    chip.setPosition(center);
    chip.setFillColor(sf::Color(10, 13, 19, 220));
    chip.setOutlineColor(withAlpha(accent, 190));
    chip.setOutlineThickness(1.2f);
    window.draw(chip);

    sf::FloatRect box = label.getLocalBounds();
    label.setOrigin(box.position + box.size / 2.f);
    label.setFillColor(accent);
    label.setPosition(center);
    window.draw(label);
}

string Theme::elide(const string& text, unsigned int size, float maxWidth) const {
    sf::Text probe(font, sf::String::fromUtf8(text.begin(), text.end()), size);
    if (probe.getLocalBounds().size.x <= maxWidth) return text;
    for (int cut = static_cast<int>(text.size()) - 1; cut > 0; cut--) {
        string candidate = text.substr(0, static_cast<std::size_t>(cut)) + "...";
        sf::Text shortened(font, sf::String::fromUtf8(candidate.begin(), candidate.end()), size);
        if (shortened.getLocalBounds().size.x <= maxWidth) return candidate;
    }
    return "";
}

void Theme::drawWrappedText(sf::RenderWindow& window, const string& text, sf::Vector2f position,
                            unsigned int size, sf::Color color, float maxWidth, float lineHeight,
                            int maxLines) const {
    if (maxLines > 8) maxLines = 8;
    if (maxLines < 1) return;

    sf::Text probe(font, "", size);
    auto widthOf = [&](const string& value) {
        probe.setString(sf::String::fromUtf8(value.begin(), value.end()));
        return probe.getLocalBounds().size.x;
    };

    string lines[8];
    int count = 0;
    string line;
    string word;
    for (std::size_t i = 0; i <= text.size(); i++) {
        char c = i < text.size() ? text[i] : ' ';
        if (c == ' ' || c == '\n') {
            if (!word.empty()) {
                string candidate = line.empty() ? word : line + " " + word;
                if (!line.empty() && widthOf(candidate) > maxWidth) {
                    lines[count++] = line;
                    if (count >= maxLines) break;
                    line = word;
                } else {
                    line = candidate;
                }
                word.clear();
            }
            if (c == '\n' && !line.empty()) {
                lines[count++] = line;
                if (count >= maxLines) break;
                line.clear();
            }
        } else {
            word += c;
        }
    }
    if (!line.empty() && count < maxLines) lines[count++] = line;

    for (int i = 0; i < count; i++) {
        sf::Text label(font, sf::String::fromUtf8(lines[i].begin(), lines[i].end()), size);
        label.setFillColor(color);
        sf::FloatRect box = label.getLocalBounds();
        label.setOrigin(box.position + box.size / 2.f);
        float offset = (static_cast<float>(i) - (static_cast<float>(count) - 1.f) / 2.f) *
                       lineHeight;
        label.setPosition({position.x, position.y + offset});
        window.draw(label);
    }
}

}
