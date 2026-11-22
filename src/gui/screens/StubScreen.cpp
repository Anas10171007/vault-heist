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
    addButton(label, "", targetScreen);
}

void StubScreen::addButton(const string& label, const string& sublabel, int targetScreen) {
    if (buttonCount >= MAX_BUTTONS) return;
    buttons[buttonCount].label = label;
    buttons[buttonCount].sublabel = sublabel;
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
    theme.drawVaultBackdrop(window);
    Renderer renderer(window, theme.getFont());

    // Brand-first hero: product name dominates the first viewport.
    theme.drawGlowTitle(window, "VAULT HEIST", {550.f, 118.f}, 54, Palette::BRASS, true);
    renderer.drawText(title == "Vault Heist" ? "THE LIVING VAULT" : title,
                      {550.f, 176.f}, 15, Palette::VISITED, true);
    renderer.drawText(description, {550.f, 206.f}, 15, Palette::MUTED, true);

    sf::RectangleShape rule({240.f, 2.f});
    rule.setPosition({430.f, 240.f});
    rule.setFillColor(Palette::BRASS);
    window.draw(rule);

    sf::RectangleShape ruleCyan({90.f, 2.f});
    ruleCyan.setPosition({505.f, 240.f});
    ruleCyan.setFillColor(Palette::VISITED);
    window.draw(ruleCyan);

    for (int i = 0; i < buttonCount; i++) {
        sf::FloatRect bounds = buttonBounds(i);
        int kind = i == 0 ? ButtonKind::PRIMARY : ButtonKind::SECONDARY;
        if (buttons[i].label == "Quit" || buttons[i].label == "Bounce") {
            kind = ButtonKind::GHOST;
        }
        theme.drawGlossyButton(window, theme.getFont(), bounds, buttons[i].label,
                               buttons[i].sublabel, kind, theme.hovered(window, bounds));
    }

    renderer.drawText("Click a card to enter  ·  Esc backs out of missions", {550.f, 660.f}, 12,
                      Palette::MUTED, true);
}

sf::FloatRect StubScreen::buttonBounds(int index) const {
    return sf::FloatRect({375.f, 280.f + index * 52.f}, {350.f, 44.f});
}

}
