#pragma once
#include <string>
#include "game/LevelDef.h"
#include "game/act4/HeapBuildLevel.h"
#include "game/act4/HeapRepairLevel.h"
#include "game/act4/HeapOperationLevel.h"
#include "gui/core/AnimationController.h"
#include "gui/core/Screen.h"
#include "gui/core/Theme.h"
#include "gui/widgets/AlarmMeter.h"
#include "gui/widgets/HeapView.h"

using namespace std;

namespace vh {

class HeapLevelScreen : public Screen {
public:
    HeapLevelScreen(Theme& theme);
    int handleEvent(const sf::Event& event, sf::RenderWindow& window) override;
    void update(float seconds) override;
    void draw(sf::RenderWindow& window) override;
    void onShow() override;
    bool selectLevel(const string& id);
    const ScoreSystem& score() const;

private:
    Theme& theme;
    LevelDef level;
    bool levelLoaded;
    HeapBuildLevel buildLogic;
    HeapRepairLevel repairLogic;
    HeapOperationLevel operationLogic;
    HeapView heapView;
    AnimationController animation;
    AlarmMeter alarmMeter;
    int firstSelection;
    bool waitingForAnimation;
    string message;

    void resetLevel();
    void selectIndex(int index);
    void chooseDirection(int direction);
    bool loadLevel(const string& id);
    bool isBuildLevel() const;
    bool isRepairLevel() const;
    bool isOperationLevel() const;
    void performOperation();
    void undoOperation();
    bool currentComplete() const;
    bool currentFailed() const;
    const ScoreSystem& currentScore() const;
    void copyCurrentValues(DynArray<int>& output) const;
    void drawButton(sf::RenderWindow& window, Renderer& renderer,
                    sf::FloatRect bounds, const string& label) const;
    sf::FloatRect upButton() const;
    sf::FloatRect downButton() const;
    sf::FloatRect actionButton() const;
    sf::FloatRect undoButton() const;
    sf::FloatRect resetButton() const;
    sf::FloatRect continueButton() const;
    sf::FloatRect menuButton() const;
};

}
