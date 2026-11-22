#pragma once
#include <string>
#include "game/LevelDef.h"
#include "game/act4/HeapBuildLevel.h"
#include "game/act4/HeapRepairLevel.h"
#include "game/act4/HeapOperationLevel.h"
#include "gui/core/AnimationController.h"
#include "gui/core/Screen.h"
#include "gui/core/Theme.h"
#include "gui/widgets/HeapView.h"
#include "gui/widgets/LevelChrome.h"
#include "gui/widgets/ReplayScrubber.h"
#include "model/EventLog.h"
#include "util/DynArray.h"

using namespace std;

namespace vh {

class HeapLevelScreen : public Screen {
public:
    HeapLevelScreen(Theme& theme);
    int handleEvent(const sf::Event& event, sf::RenderWindow& window) override;
    int update(float seconds) override;
    void draw(sf::RenderWindow& window) override;
    void onShow() override;
    bool selectLevel(const string& id);
    const ScoreSystem& score() const;
    void setAnimationSpeed(float speed);
    bool isFinalVaultLevel() const;
    string sortedArrayText() const;
    bool canReviewReplay() const;
    bool startReviewReplay();
    bool isReviewMode() const;

private:
    Theme& theme;
    LevelDef level;
    bool levelLoaded;
    HeapBuildLevel buildLogic;
    HeapRepairLevel repairLogic;
    HeapOperationLevel operationLogic;
    HeapView heapView;
    AnimationController animation;
    LevelChrome chrome;
    ReplayScrubber scrubber;
    EventLog levelTimeline;
    DynArray<int> replayBaseline;
    int replayBaselineActive;
    int firstSelection;
    bool waitingForAnimation;
    bool extractDragging;
    bool reviewMode;
    string message;

    void resetLevel();
    void selectIndex(int index);
    void playOperationEvents(const EventLog& events);
    void resetReplayTimeline();
    void confirmPendingExtract();
    void beginSoloExtract();
    void runSelectedHeapify(bool siftUp);
    void runBuildAction();
    void runHeapifyRootAction();
    bool canSoloExtract() const;
    bool loadLevel(const string& id);
    bool isBuildLevel() const;
    bool isRepairLevel() const;
    bool isOperationLevel() const;
    void undoOperation();
    void showHint();
    void adjustSpeed(bool faster);
    bool currentComplete() const;
    bool currentFailed() const;
    const ScoreSystem& currentScore() const;
    string objectiveText() const;
    void copyCurrentValues(DynArray<int>& output) const;
};

}
