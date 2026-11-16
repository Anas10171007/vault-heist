#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include "gui/core/Camera.h"
#include "gui/core/Renderer.h"
int main() {
    sf::RenderWindow window(sf::VideoMode({1000u, 650u}), "Renderer test");
    window.setFramerateLimit(60);
    sf::Font font;
    if (!loadDefaultFont(font)) {
        std::cout << "font not found" << std::endl;
        return 1;
    }
    Renderer renderer(window, font);
    Camera camera(sf::Vector2f(window.getSize()));
    sf::FloatRect treeBounds({-90.f, -50.f}, {820.f, 560.f});
    sf::FloatRect stressBounds({0.f, 0.f}, {1400.f, 700.f});
    bool stress = false;
    camera.fitToView(treeBounds, 40.f);
    sf::RectangleShape frame;
    frame.setFillColor(Palette::PANEL);
    frame.setOutlineColor(Palette::BORDER);
    frame.setOutlineThickness(4.f);
    sf::Clock clock;
    int frames = 0;
    while (window.isOpen()) {
        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();
            camera.handleEvent(*event);
            if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
                if (key->code == sf::Keyboard::Key::Escape) window.close();
                if (key->code == sf::Keyboard::Key::S) stress = !stress;
                if (key->code == sf::Keyboard::Key::F || key->code == sf::Keyboard::Key::S) camera.fitToView(stress ? stressBounds : treeBounds, 40.f);
            }
        }
        sf::FloatRect current = stress ? stressBounds : treeBounds;
        frame.setPosition(current.position);
        frame.setSize(current.size);
        window.clear(Palette::BG);
        camera.apply(window);
        window.draw(frame);
        if (stress) {
            for (int i = 0; i < 200; i++) {
                sf::Vector2f p(35.f + static_cast<float>(i % 20) * 70.f, 35.f + static_cast<float>(i / 20) * 70.f);
                if (i % 20 < 19) renderer.drawEdge(p, {p.x + 70.f, p.y}, i % 5 == 0);
            }
            for (int i = 0; i < 200; i++) {
                sf::Vector2f p(35.f + static_cast<float>(i % 20) * 70.f, 35.f + static_cast<float>(i / 20) * 70.f);
                renderer.drawNode(p, std::to_string(i), i % 7);
            }
        } else {
            sf::Vector2f pos[15];
            int hover = -1;
            sf::Vector2f mouse = camera.toWorld(sf::Mouse::getPosition(window));
            int first = 0, count = 1, level = 0;
            for (int i = 0; i < 15; i++) {
                if (i >= first + count) {
                    first += count;
                    count *= 2;
                    level++;
                }
                pos[i] = {(static_cast<float>(i - first) + 0.5f) * 640.f / static_cast<float>(count), static_cast<float>(level) * 90.f};
                if ((mouse - pos[i]).length() <= Renderer::NODE_HIT_RADIUS) hover = i;
            }
            for (int i = 1; i < 15; i++) renderer.drawEdge(pos[(i - 1) / 2], pos[i], i == hover);
            for (int i = 0; i < 15; i++) renderer.drawNode(pos[i], std::to_string(10 + i * 5), i % 7);
            for (int i = 0; i < 7; i++) {
                int b = i % 3 - 1;
                renderer.drawBadge(pos[i] + sf::Vector2f(26.f, -26.f), b > 0 ? "+1" : (b == 0 ? "0" : "-1"), b == 0 ? NodeState::BALANCED : NodeState::WARNING);
            }
            for (int i = 0; i < 15; i++) renderer.drawArrayCell({-58.f + static_cast<float>(i) * 54.f, 400.f}, i, 10 + i * 5, i < 10 ? CellRegion::ACTIVE : (i < 12 ? CellRegion::SWAPPING : CellRegion::SORTED));
            renderer.drawText("ACTIVE", {185.f, 362.f}, 12, Palette::MUTED, true);
            renderer.drawText("SWAP", {509.f, 362.f}, 12, Palette::MUTED, true);
            renderer.drawText("SORTED", {644.f, 362.f}, 12, Palette::MUTED, true);
            renderer.drawText(hover < 0 ? "hover a node" : "hover: node " + std::to_string(10 + hover * 5), {-80.f, -40.f}, 16, Palette::TEXT, false);
        }
        frames++;
        if (clock.getElapsedTime().asSeconds() >= 0.5f) {
            window.setTitle("Renderer test - " + std::string(stress ? "200 nodes" : "15 nodes") + " - fps " + std::to_string(static_cast<int>(static_cast<float>(frames) / clock.restart().asSeconds())));
            frames = 0;
        }
        window.display();
    }
    return 0;
}