#include "gui/screens/StubScreen.h"

namespace vh {

StubScreen::StubScreen(Theme& theme, const string& title, const string& description)
    : theme(theme) {
    this->title = title;
    this->description = description;
    buttonCount = 0;
}

void StubScreen::addButton(const string& label, int targetScreen) {
    if (buttonCount >= MAX_BUTTONS) {
        return;
    }

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

void StubScreen::update(float deltaSeconds) {
    (void)deltaSeconds;
}

void StubScreen::draw(sf::RenderWindow& window) {
    sf::RectangleShape panel = theme.makePanel({220.f, 75.f}, {660.f, 570.f});
    window.draw(panel);

    sf::Text titleLabel = theme.makeLabel(title, 42, theme.gold);
    sf::FloatRect titleSize = titleLabel.getLocalBounds();
    titleLabel.setOrigin({titleSize.position.x + titleSize.size.x / 2.f, titleSize.position.y});
    titleLabel.setPosition({550.f, 105.f});
    window.draw(titleLabel);

    sf::Text stubLabel = theme.makeLabel("STUB SCREEN", 16, theme.yellow);
    sf::FloatRect stubSize = stubLabel.getLocalBounds();
    stubLabel.setOrigin({stubSize.position.x + stubSize.size.x / 2.f, stubSize.position.y});
    stubLabel.setPosition({550.f, 165.f});
    window.draw(stubLabel);

    sf::Text descriptionLabel = theme.makeLabel(description, 20, theme.text);
    sf::FloatRect descriptionSize = descriptionLabel.getLocalBounds();
    descriptionLabel.setOrigin({descriptionSize.position.x + descriptionSize.size.x / 2.f,
                                descriptionSize.position.y});
    descriptionLabel.setPosition({550.f, 205.f});
    window.draw(descriptionLabel);

    for (int i = 0; i < buttonCount; i++) {
        sf::FloatRect bounds = buttonBounds(i);
        sf::RectangleShape button = theme.makeButton(bounds.position, bounds.size);
        window.draw(button);

        sf::Text buttonLabel = theme.makeLabel(buttons[i].label, 22, theme.text);
        sf::FloatRect labelSize = buttonLabel.getLocalBounds();
        buttonLabel.setOrigin({labelSize.position.x + labelSize.size.x / 2.f,
                               labelSize.position.y + labelSize.size.y / 2.f});
        buttonLabel.setPosition({bounds.position.x + bounds.size.x / 2.f,
                                 bounds.position.y + bounds.size.y / 2.f - 4.f});
        window.draw(buttonLabel);
    }

    sf::Text help = theme.makeLabel("Click a button to change screen  |  Esc: Main Menu", 16, theme.grey);
    sf::FloatRect helpSize = help.getLocalBounds();
    help.setOrigin({helpSize.position.x + helpSize.size.x / 2.f, helpSize.position.y});
    help.setPosition({550.f, 605.f});
    window.draw(help);
}

sf::FloatRect StubScreen::buttonBounds(int index) const {
    return sf::FloatRect({350.f, 255.f + index * 55.f}, {400.f, 44.f});
}

}
