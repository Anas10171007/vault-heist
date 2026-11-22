#pragma once
#include <string>
#include "game/tree/TreeLayout.h"
#include "gui/core/Screen.h"
#include "gui/core/Theme.h"
#include "gui/tree/TreeAnimationView.h"
#include "model/tree/AVLTree.h"
#include "util/DynArray.h"
#include "util/Rng.h"

using namespace std;

namespace vh {

class EndlessScreen : public Screen {
public:
    explicit EndlessScreen(Theme& theme);

    int handleEvent(const sf::Event& event, sf::RenderWindow& window) override;
    int update(float deltaSeconds) override;
    void draw(sf::RenderWindow& window) override;
    void onShow() override;

    // Session-local scores kept for tests; durable scores go through Leaderboard.
    const DynArray<int>& leaderboard() const { return scores; }
    bool isEnded() const { return ended; }

private:
    struct EndlessOp {
        bool insert = false;
        int value = 0;
    };

    void refreshView();
    float delay() const;
    void generateOp();
    void applyPending();
    void finishRun();
    bool isValid() const;
    static bool contains(sf::FloatRect bounds, sf::Vector2f point);
    static sf::FloatRect menuButton();
    static sf::FloatRect restartButton();

    Theme& theme;
    AVLTree avl;
    TreeAnimationView view;
    Rng rng;
    EndlessOp pending;
    bool pendingHas = false;
    float elapsed = 0.f;
    float nextOpTimer = 1.5f;
    int ops = 0;
    int typedValue = 0;
    bool ended = false;
    bool postedScore = false;
    int lastScore = 0;
    DynArray<int> scores;
};

}
