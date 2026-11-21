#pragma once

#include <string>

#include "game/LevelDef.h"
#include "game/ScoreSystem.h"
#include "model/EventLog.h"
#include "model/Event.h"
#include "model/tree/BinaryTree.h"

namespace vh {

// Action kinds are plain integers so the game logic stays free of enums and
// SFML. The GUI translates a click into one of these values.
struct ActionKind {
    static constexpr int CLICK_NODE = 0;
    static constexpr int PLACE_TOKEN = 1;
    static constexpr int ANSWER = 2;
    static constexpr int CHOOSE_CASE = 3;
    static constexpr int CONFIRM = 4;
    static constexpr int AUTO = 5;
};

struct DeleteCase {
    static constexpr int LEAF = 0;
    static constexpr int ONE_CHILD = 1;
    static constexpr int TWO_CHILD = 2;
};

struct PlayerAction {
    int kind = ActionKind::CLICK_NODE;
    int id = 0;       // node id: clicked node or rotation pivot
    int value = 0;    // placed token or numeric answer
    int slot = 0;     // reconstruction slot index
    int choice = 0;   // delete case, RotationType constant, or -1 for no rotation

    PlayerAction() = default;
};

inline PlayerAction clickNode(int id) {
    PlayerAction action;
    action.kind = ActionKind::CLICK_NODE;
    action.id = id;
    return action;
}

inline PlayerAction placeToken(int slot, int value) {
    PlayerAction action;
    action.kind = ActionKind::PLACE_TOKEN;
    action.slot = slot;
    action.value = value;
    return action;
}

inline PlayerAction answer(int value) {
    PlayerAction action;
    action.kind = ActionKind::ANSWER;
    action.value = value;
    return action;
}

inline PlayerAction chooseCase(int choice) {
    PlayerAction action;
    action.kind = ActionKind::CHOOSE_CASE;
    action.choice = choice;
    return action;
}

inline PlayerAction confirmAction() {
    PlayerAction action;
    action.kind = ActionKind::CONFIRM;
    return action;
}

inline PlayerAction autoAction() {
    PlayerAction action;
    action.kind = ActionKind::AUTO;
    return action;
}

struct PlayResult {
    bool correct = false;
    bool wrong = false;
    bool won = false;
    bool lost = false;
    std::string message;
    EventLog events;
    bool hasHint = false;
    PlayerAction hintAction;

    PlayResult() = default;
    PlayResult(PlayResult&&) = default;
    PlayResult& operator=(PlayResult&&) = default;
    PlayResult(const PlayResult&) = delete;
    PlayResult& operator=(const PlayResult&) = delete;
};

// One small interface for every act. The screens own a LevelPlay*, call load(),
// draw tree(), feed clicks to apply(), and read score() for the alarm meter.
class LevelPlay {
public:
    virtual ~LevelPlay() = default;

    virtual bool load(const LevelDef& definition) = 0;
    virtual void reset() = 0;
    virtual PlayResult apply(const PlayerAction& action) = 0;
    virtual PlayResult hint() = 0;
    // Snapshot undo: restore tree + score + progress from before the last apply.
    virtual bool undo() { return false; }

    // Free correct next step, used by Auto, tests and tutorial highlighting.
    virtual PlayerAction nextAction() const = 0;
    virtual int optimalSteps() const = 0;
    virtual bool isComplete() const = 0;
    virtual bool isFailed() const = 0;
    virtual const ScoreSystem& score() const = 0;
    virtual const BinaryTree& tree() const = 0;
    virtual const LevelDef& definition() const = 0;
};

}
