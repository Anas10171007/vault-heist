#include <SFML/Graphics.hpp>
#include <direct.h>
#include <iostream>
#include <string>

#include "game/tree/TreeLayout.h"
#include "game/tree/TreeMotion.h"
#include "gui/core/AnimationController.h"
#include "gui/core/Renderer.h"
#include "gui/core/Style.h"
#include "gui/tree/TreeAnimationView.h"
#include "model/tree/AVLTree.h"

using namespace std;
using namespace vh;

namespace {

const char* scenarioName(int index) {
    if (index == 0) return "LL";
    if (index == 1) return "RR";
    if (index == 2) return "LR";
    return "RL";
}

void playInsert(AVLTree& tree, TreeAnimationView& view, AnimationController& animation, int key,
                const char* label, int& firstRotate, int& lastRotate) {
    TreeLayout before = TreeLayout::compute(tree);
    BST::OpResult result = tree.insert(key);
    TreeLayout after = TreeLayout::compute(tree);
    firstRotate = -1;
    lastRotate = -1;
    int rotateCount = 0;
    for (int i = 0; i < result.events.size(); i++) {
        if (result.events.at(i).type == EventType::ROTATE) {
            if (firstRotate < 0) firstRotate = i;
            lastRotate = i;
            rotateCount++;
        }
    }
    view.setModel(tree);
    view.setMotion(TreeMotion::simple(before, result.events, after));
    animation.load(result.events);
    animation.play();
    cout << "play " << label << ": ok=" << result.ok << " events=" << result.events.size()
         << " nodes=" << tree.size() << " rotations=" << rotateCount << endl;
}

void loadScenario(int index, AVLTree& tree, TreeAnimationView& view,
                  AnimationController& animation, int& firstRotate, int& lastRotate) {
    int bases[4][2] = {{30, 20}, {10, 20}, {30, 10}, {10, 30}};
    int keys[4] = {10, 30, 20, 20};
    tree.clear();
    tree.insert(bases[index][0]);
    tree.insert(bases[index][1]);
    playInsert(tree, view, animation, keys[index], scenarioName(index), firstRotate, lastRotate);
}

void saveScreenshot(sf::RenderWindow& window, TreeAnimationView& view, const sf::Font& font,
                    const char* path) {
    _mkdir("screenshots");
    sf::RenderTexture target(window.getSize());
    Renderer capture(target, font);
    target.clear(Palette::BG);
    view.draw(capture);
    capture.drawText("TREE ANIMATION - LL / RR / LR / RL", {550.f, 686.f}, 14, Palette::MUTED, true);
    target.display();
    (void)target.getTexture().copyToImage().saveToFile(path);
    cout << "screenshot saved to " << path << endl;
}

}

int main(int argc, char** argv) {
    bool smoke = argc > 1 && string(argv[1]) == "--smoke";
    bool shots = argc > 1 && string(argv[1]) == "--shots";

    sf::RenderWindow window(sf::VideoMode({1100u, 720u}), "Tree Anim demo");
    window.setFramerateLimit(60);

    sf::Font font;
    if (!loadDefaultFont(font)) {
        cout << "font not found" << endl;
        return 1;
    }

    AVLTree tree;
    TreeAnimationView view;
    AnimationController animation;
    view.setOrigin(70.f, 110.f);
    animation.setListener(&view);
    int firstRotate = -1;
    int lastRotate = -1;
    loadScenario(0, tree, view, animation, firstRotate, lastRotate);

    int insertKeys[6] = {5, 95, 25, 65, 45, 85};
    int insertIndex = 0;
    Renderer renderer(window, font);
    sf::Clock clock;

    while (window.isOpen()) {
        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();
            if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
                if (key->code == sf::Keyboard::Key::Escape) window.close();
                if (key->code == sf::Keyboard::Key::Num1) loadScenario(0, tree, view, animation, firstRotate, lastRotate);
                if (key->code == sf::Keyboard::Key::Num2) loadScenario(1, tree, view, animation, firstRotate, lastRotate);
                if (key->code == sf::Keyboard::Key::Num3) loadScenario(2, tree, view, animation, firstRotate, lastRotate);
                if (key->code == sf::Keyboard::Key::Num4) loadScenario(3, tree, view, animation, firstRotate, lastRotate);
                if (key->code == sf::Keyboard::Key::I) {
                    int next = insertKeys[insertIndex % 6];
                    insertIndex++;
                    playInsert(tree, view, animation, next, "INSERT", firstRotate, lastRotate);
                }
                if (key->code == sf::Keyboard::Key::D && tree.size() > 0) {
                    BSTNode* minimum = tree.findMin();
                    int keyToDelete = minimum == nullptr ? 0 : minimum->key;
                    TreeLayout before = TreeLayout::compute(tree);
                    BST::OpResult result = tree.deleteKey(keyToDelete);
                    TreeLayout after = TreeLayout::compute(tree);
                    view.setModel(tree);
                    view.setMotion(TreeMotion::simple(before, result.events, after));
                    animation.load(result.events);
                    animation.play();
                }
                if (key->code == sf::Keyboard::Key::Space) {
                    if (animation.isPlaying()) animation.pause();
                    else animation.play();
                }
                if (key->code == sf::Keyboard::Key::Right) animation.stepForward();
                if (key->code == sf::Keyboard::Key::Left) animation.stepBackward();
                if (key->code == sf::Keyboard::Key::P) {
                        saveScreenshot(window, view, font, "screenshots/tree_anim.png");
                    }
            }
        }

        animation.update(clock.restart().asSeconds());
        window.clear(Palette::BG);
        view.draw(renderer);
        renderer.drawText("1-4 LL/RR/LR/RL   I insert   D delete   SPACE play/pause",
                          {550.f, 686.f}, 14, Palette::MUTED, true);
        renderer.drawText("LEFT/RIGHT step   P screenshot   ESC quit",
                          {550.f, 706.f}, 14, Palette::MUTED, true);
        renderer.drawText("EVENT " + to_string(animation.completedCount()) + " / " +
                              to_string(animation.eventCount()),
                          {550.f, 28.f}, 16, Palette::TEXT, true);
        window.display();

        if (smoke) {
            saveScreenshot(window, view, font, "screenshots/tree_anim.png");
            window.close();
            cout << "smoke ok" << endl;
            return 0;
        }
        if (shots) {
            const char* paths[4] = {"screenshots/rotation_LL.png", "screenshots/rotation_RR.png",
                                    "screenshots/rotation_LR.png", "screenshots/rotation_RL.png"};
            for (int scenario = 0; scenario < 4; scenario++) {
                int first = -1;
                int last = -1;
                loadScenario(scenario, tree, view, animation, first, last);
                if (first < 0) {
                    animation.finishInstantly();
                    saveScreenshot(window, view, font, paths[scenario]);
                    continue;
                }
                for (int step = 0; step < first; step++) animation.stepForward();
                string beforePath = string(paths[scenario]).substr(0, string(paths[scenario]).size() - 4) +
                                    "_before.png";
                saveScreenshot(window, view, font, beforePath.c_str());
                for (int step = first; step <= last; step++) animation.stepForward();
                saveScreenshot(window, view, font, paths[scenario]);
            }
            window.close();
            cout << "four rotation screenshots saved" << endl;
            return 0;
        }
    }

    return 0;
}
