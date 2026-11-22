#pragma once

#include "game/LevelManager.h"
#include "game/LevelPlay.h"
#include "gui/core/AnimationController.h"
#include "gui/core/Theme.h"
#include "gui/tree/TreeAnimationView.h"
#include "gui/widgets/LevelChrome.h"
#include "util/DynArray.h"

using namespace std;

namespace vh {

// Vault-chrome presentation of Anas's LevelPlay (Acts 1–3).
// His LevelPlayScreen, tree view, and act logic are not modified.
class TreeMissionScreen {
public:
    explicit TreeMissionScreen(Theme& theme);
    ~TreeMissionScreen();

    bool loadLevel(const string& id);
    int handleEvent(const sf::Event& event, sf::RenderWindow& window);
    int update(float seconds);
    void draw(sf::RenderWindow& window);

    bool hasLevel() const { return logic != nullptr; }
    bool failed() const { return logic != nullptr && logic->isFailed(); }
    const ScoreSystem& score() const { return logic->score(); }
    const string& statusMessage() const { return message; }
    static string objectiveFor(const LevelDef& def);

private:
    struct UiUndo {
        int markCount = 0;
        int slotIndex = -1;
        int previousValue = 0;
        int previousFilled = 0;
        int selectedNode = 0;
        int selectedToken = -1;
        int answer = 0;
    };

    class Bridge : public AnimationListener {
    public:
        void reset() override {
            heard = false;
            progress = 0.f;
        }
        void onEvent(const Event& event, float amount) override {
            (void)event;
            progress = amount;
            heard = true;
        }
        bool heard = false;
        float progress = 0.f;
    };

    static sf::FloatRect playfield();
    static sf::FloatRect tokenRect(int index);
    static sf::FloatRect slotRect(int index);
    static sf::FloatRect caseRect(int index, float width);
    static sf::FloatRect submitRect();

    void refreshTokens();
    void rebuildStill();
    void fitBox(float minX, float minY, float maxX, float maxY);
    void fitLayout(const TreeLayout& layout);
    void fitUnion(const TreeLayout& before, const TreeLayout& after);
    void syncView();
    void remember(const EventLog& events, const TreeLayout& layout);
    void trimMarks(int count);
    bool marked(int nodeId) const;
    void pushUndo(const PlayerAction& action);
    void apply(const PlayerAction& action);
    void undo();
    void resetLevel();
    void useHint();
    void armResult();
    string instruction() const;
    string describeAction(const PlayerAction& action) const;
    string caseName(int choice) const;
    sf::Vector2f worldAt(sf::Vector2f screen) const;
    void applyPlayfieldView(sf::RenderWindow& window) const;
    void drawRings(sf::RenderWindow& window) const;
    void drawActions(sf::RenderWindow& window, Renderer& renderer);
    bool handleToolbar(sf::Vector2f point);
    bool handleActions(sf::Vector2f point);
    void handleNode(sf::Vector2f point);
    void clearPan();
    int screenTarget;

    Theme& theme;
    LevelChrome chrome;
    LevelManager manager;
    LevelPlay* logic = nullptr;
    TreeAnimationView treeView;
    Bridge bridge;
    AnimationController animation;
    DynArray<LevelSnapshot> undoStack;
    DynArray<UiUndo> undoUi;
    DynArray<int> tokens;
    DynArray<int> slotValue;
    DynArray<int> slotFilled;
    DynArray<int> marks;
    string levelId;
    string message;
    sf::Vector2f center;
    float scale = 1.f;
    sf::Vector2f panLast;
    bool panning = false;
    bool panMoved = false;
    int selectedNode = 0;
    int wrongNode = 0;
    int selectedToken = -1;
    int selectedSlot = -1;
    int answer = 0;
    bool pendingResult = false;
};
}
