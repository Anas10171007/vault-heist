#include "gui/screens/StubScreen.h"
#include "gui/core/Renderer.h"

namespace vh {

StubScreen::StubScreen(Theme& theme, const string& title, const string& description)
    : theme(theme) {
    this->title = title;
    this->description = description;
    buttonCount = 0;
}

void StubScreen::addButton(const string& label, int targetScreen) {
    if (buttonCount >= MAX_BUTTONS) return;
    buttons[buttonCount].label = label;
    buttons[buttonCount].targetScreen = targetScreen;
    buttonCount++;
}

int StubScreen::handleEvent(const sf::Event& event, sf::RenderWindow& window) {
    const sf::Event::MouseButtonPressed* mouseClick = event.getIf<sf::Event::MouseButtonPressed>();
    if (mouseClick == nullptr || mouseClick->button != sf::Mouse::Button::Left) {
        return ScreenId::NONE;
    }

    sf::Vector2f mousePosition = window.mapPixelToCoords(mouseClick->position);
    for (int i = 0; i < buttonCount; i++) {
        if (buttonBounds(i).contains(mousePosition)) {
            return buttons[i].targetScreen;
        }
    }
    return ScreenId::NONE;
}

int StubScreen::update(float deltaSeconds) {
    (void)deltaSeconds;
    return ScreenId::NONE;
}

void StubScreen::draw(sf::RenderWindow& window) {
    theme.drawBackdrop(window);
    Renderer renderer(window, theme.getFont());

    // Brand-first hero: product name dominates the first viewport.
    renderer.drawText("VAULT HEIST", {550.f, 120.f}, 54, Palette::BRASS, true);
    renderer.drawText(title == "Vault Heist" ? "THE LIVING VAULT" : title,
                      {550.f, 178.f}, 18, Palette::TEXT, true);
    renderer.drawText(description, {550.f, 214.f}, 15, Palette::MUTED, true);

    sf::RectangleShape rule({220.f, 2.f});
    rule.setPosition({440.f, 246.f});
    rule.setFillColor(Palette::BRASS);
    window.draw(rule);

    for (int i = 0; i < buttonCount; i++) {
        sf::FloatRect bounds = buttonBounds(i);
        int kind = i == 0 ? ButtonKind::PRIMARY : ButtonKind::SECONDARY;
        if (buttons[i].label == "Quit") kind = ButtonKind::GHOST;
        theme.drawButton(window, theme.getFont(), bounds, buttons[i].label, kind,
                         theme.hovered(window, bounds));
    }

    renderer.drawText("Click to enter  ·  Esc returns to menu", {550.f, 660.f}, 13,
                      Palette::MUTED, true);
}

sf::FloatRect StubScreen::buttonBounds(int index) const {
    return sf::FloatRect({375.f, 280.f + index * 52.f}, {350.f, 44.f});
}

}
