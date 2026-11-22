#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include "gui/core/Style.h"

using namespace std;

namespace vh {

class Theme {
public:
    Theme();
    bool loadFont();
    const sf::Font& getFont() const;
    float timeSeconds() const;

    sf::Text makeLabel(const string& text, unsigned int size, sf::Color color) const;
    sf::RectangleShape makePanel(sf::Vector2f position, sf::Vector2f size) const;
    sf::RectangleShape makeButton(sf::Vector2f position, sf::Vector2f size) const;
    sf::RectangleShape makeButton(sf::Vector2f position, sf::Vector2f size, int kind,
                                  bool hovered) const;

    void drawBackdrop(sf::RenderWindow& window) const;
    void drawPanel(sf::RenderWindow& window, sf::FloatRect bounds) const;
    void drawButton(sf::RenderWindow& window, const sf::Font& font, sf::FloatRect bounds,
                    const string& label, int kind, bool hovered) const;
    bool hovered(const sf::RenderWindow& window, sf::FloatRect bounds) const;

    // Neon-vault shell visuals. Added for the menu, campaign, briefing, result
    // and settings screens only; the level screens keep the original helpers.
    void drawVaultBackdrop(sf::RenderWindow& window) const;
    void drawGlowTitle(sf::RenderWindow& window, const string& text, sf::Vector2f position,
                       unsigned int size, sf::Color color, bool centered) const;
    void drawGlossyButton(sf::RenderWindow& window, const sf::Font& font, sf::FloatRect bounds,
                          const string& label, int kind, bool hovered) const;
    void drawGlossyButton(sf::RenderWindow& window, const sf::Font& font, sf::FloatRect bounds,
                          const string& label, const string& sublabel, int kind,
                          bool hovered) const;
    void drawCard(sf::RenderWindow& window, sf::FloatRect bounds, sf::Color accent,
                  bool hovered) const;
    void drawStar(sf::RenderWindow& window, sf::Vector2f center, float radius,
                  bool filled) const;
    void drawChip(sf::RenderWindow& window, sf::Vector2f center, const string& text,
                  unsigned int size, sf::Color accent) const;
    string elide(const string& text, unsigned int size, float maxWidth) const;
    void drawWrappedText(sf::RenderWindow& window, const string& text, sf::Vector2f position,
                         unsigned int size, sf::Color color, float maxWidth, float lineHeight,
                         int maxLines) const;

private:
    sf::Font font;
    sf::Clock clock;
    bool tryLoad(const string& path);
};

}
