#include <SFML/Graphics.hpp>
#include <iostream>
#include "gui/core/AnimationController.h"
#include "gui/core/Renderer.h"
#include "gui/widgets/HeapView.h"
#include "model/heap/MaxHeap.h"

using namespace std;
using namespace vh;

int main() {
    sf::RenderWindow window(sf::VideoMode({1100u, 760u}), "Heap View demo");
    window.setFramerateLimit(60);

    sf::Font font;
    if (!loadDefaultFont(font)) {
        cout << "font not found" << endl;
        return 1;
    }

    DynArray<int> start;
    int sample[15] = {18, 72, 35, 90, 41, 63, 12, 55, 29, 84, 7, 46, 68, 21, 77};
    for (int index = 0; index < 15; index++) start.push(sample[index]);

    MaxHeap heap;
    heap.load(start);
    EventLog events = heap.heapSort();

    HeapView view;
    view.load(start, start.size());
    AnimationController animation;
    animation.setListener(&view);
    animation.load(events);

    Renderer renderer(window, font);
    sf::Clock clock;

    while (window.isOpen()) {
        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();
            if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
                if (key->code == sf::Keyboard::Key::Escape) window.close();
                if (key->code == sf::Keyboard::Key::Space) {
                    if (animation.isPlaying()) animation.pause();
                    else animation.play();
                }
                if (key->code == sf::Keyboard::Key::Right) animation.stepForward();
                if (key->code == sf::Keyboard::Key::Left) animation.stepBackward();
                if (key->code == sf::Keyboard::Key::R) animation.restart();
            }
        }

        animation.update(clock.restart().asSeconds());
        window.clear(Palette::BG);
        view.draw(renderer, {45.f, 100.f}, 1010.f, {45.f, 545.f}, 1010.f);
        renderer.drawText("SPACE play/pause   LEFT/RIGHT step   R reset   ESC close",
                          {550.f, 725.f}, 14, Palette::MUTED, true);
        renderer.drawText("EVENT " + to_string(animation.completedCount()) +
                          " / " + to_string(animation.eventCount()),
                          {550.f, 25.f}, 14, Palette::TEXT, true);
        window.display();
    }

    return 0;
}
