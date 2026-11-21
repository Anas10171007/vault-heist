#include <SFML/Graphics.hpp>
#include <direct.h>
#include <iostream>
#include <string>

#include "gui/core/Renderer.h"
#include "gui/core/Theme.h"
#include "gui/screens/CampaignMapScreen.h"
#include "gui/screens/LevelPlayScreen.h"

using namespace std;
using namespace vh;

namespace {

void capture(sf::RenderTexture& target, const char* path) {
    _mkdir("screenshots");
    (void)target.getTexture().copyToImage().saveToFile(path);
    cout << "screenshot saved to " << path << endl;
}

}

int main(int argc, char** argv) {
    bool logicOnly = argc > 1 && string(argv[1]) == "--logic";

    SessionProgress progress;
    DynArray<string> failures;
    bool allLevels = LevelPlayScreen::playAllLevels(progress, failures);
    cout << "logic smoke: " << (allLevels ? "PASS" : "FAIL") << " ("
         << progress.totalStars() << " stars)" << endl;
    for (int i = 0; i < failures.size(); i++) cout << "  " << failures[i] << endl;

    if (logicOnly) return allLevels ? 0 : 1;

    sf::RenderWindow window(sf::VideoMode({1100u, 720u}), "Level smoke");
    Theme theme;
    if (!theme.loadFont()) {
        cout << "font not found" << endl;
        return 1;
    }

    LevelPlayScreen play(theme);
    CampaignMapScreen map(theme, &play, play.session());

    sf::RenderTexture target(window.getSize());
    target.clear(Palette::BG);
    map.drawTo(target);
    target.display();
    capture(target, "screenshots/campaign_map.png");

    const char* ids[3] = {"1.1", "2.1", "3.1"};
    const char* paths[3] = {"screenshots/act1_level.png", "screenshots/act2_level.png",
                            "screenshots/act3_level.png"};
    for (int i = 0; i < 3; i++) {
        if (!play.loadLevel(ids[i])) {
            cout << "could not load " << ids[i] << endl;
            allLevels = false;
            continue;
        }
        target.clear(Palette::BG);
        play.drawTo(target);
        target.display();
        capture(target, paths[i]);
    }

    cout << "window smoke: " << (allLevels ? "PASS" : "FAIL") << endl;
    window.close();
    return allLevels ? 0 : 1;
}
