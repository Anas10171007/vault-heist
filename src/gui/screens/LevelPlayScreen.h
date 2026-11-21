#pragma once

#include <string>

#include "game/LevelDef.h"
#include "game/LevelManager.h"
#include "game/LevelPlay.h"
#include "game/tree/TreeLayout.h"
#include "game/tree/TreeMotion.h"
#include "gui/core/AnimationController.h"
#include "gui/core/Camera.h"
#include "gui/core/Renderer.h"
#include "gui/core/Screen.h"
#include "gui/core/ScreenId.h"
#include "gui/core/Theme.h"
#include "gui/screens/SessionProgress.h"
#include "gui/tree/TreeAnimationView.h"
#include "model/Event.h"
#include "util/DynArray.h"

namespace vh {

class LevelPlayScreen : public Screen {
public:
    explicit LevelPlayScreen(Theme& theme);
    ~LevelPlayScreen() override;

    int handleEvent(const sf::Event& event, sf::RenderWindow& window) override;
    void update(float deltaSeconds) override;
    void draw(sf::RenderWindow& window) override;
    void drawTo(sf::RenderTarget& target);
    void onShow() override;

    bool loadLevel(const std::string& id);
    void setNextLevel(const std::string& id);
    SessionProgress& session() { return progress; }
    bool hasLevel() const { return logic != nullptr; }
    const std::string& currentLevelId() const { return levelId; }

    // Window-free scripted play used by the smoke option and tests.
    static bool playAllLevels(SessionProgress& progress, DynArray<std::string>& failures);

private:
    struct RectButton {
        sf::FloatRect bounds;
        std::string label;
    };

    void rebuildView();
    void fitCamera();
    void applyAction(const PlayerAction& action);
    void undoAction();
    void resetLevel();
    void useHint();
    void appendEvents(const EventLog& events);
    void refreshTokens();
    std::string describe(const Event& event) const;
    std::string actionLabel(const PlayerAction& action) const;
    void drawHud(Renderer& renderer, sf::RenderTarget& target);
    void drawButtons(Renderer& renderer, sf::RenderTarget& target,
                     const RectButton* buttons, int count) const;
    void drawOverlay(Renderer& renderer, sf::RenderTarget& target);
    bool handleButtonClick(sf::Vector2f point, int& handled);

    Theme& theme;
    SessionProgress progress;
    LevelManager manager;
    LevelPlay* logic = nullptr;
    TreeAnimationView treeView;
    AnimationController animation;
    Camera camera;
    DynArray<LevelSnapshot> undoStack;
    DynArray<std::string> logLines;
    DynArray<int> tokens;
    std::string levelId;
    std::string nextLevelId;
    std::string message;
    std::string hintText;
    int selectedNode = 0;
    int selectedToken = -1;
    int selectedSlot = -1;
    int answer = 0;
    bool answerReady = false;
    bool overlay = false;
    float speed = 1.f;
};

}
