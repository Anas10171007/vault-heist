#include <SFML/Graphics.hpp>
#include <iostream>
#include "gui/core/ScreenManager.h"
#include "gui/core/Theme.h"

using namespace std;
using namespace vh;

int main() {
    sf::RenderWindow window(sf::VideoMode({1100, 720}), "Vault Heist: The Living Vault");
    window.setFramerateLimit(60);
    window.setKeyRepeatEnabled(false);

    Theme theme;
    if (!theme.loadFont()) {
        cout << "Could not load UI font from assets/fonts/" << endl;
        return 1;
    }

    ScreenManager screenManager(theme);
    sf::Clock clock;

    while (window.isOpen()) {
        // SFML 3 returns an optional event; auto keeps that library type out of our includes.
        while (auto event = window.pollEvent()) {
            screenManager.handleEvent(*event, window);
            if (!window.isOpen()) break;
        }

        if (!window.isOpen()) break;

        float deltaSeconds = clock.restart().asSeconds();
        screenManager.update(deltaSeconds);

        window.clear(Palette::BG);
        screenManager.draw(window);
        window.display();
    }

    return 0;
}
