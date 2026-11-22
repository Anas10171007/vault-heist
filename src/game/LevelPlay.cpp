#include "game/LevelPlay.h"

namespace vh {

LevelSnapshot LevelPlay::captureSnapshot() const {
    LevelSnapshot snapshot;
    snapshot.tree = tree().captureTree();
    snapshot.score = score().capture();
    snapshot.actions = history.actions;
    snapshot.hints = history.hints;
    return snapshot;
}

void LevelPlay::restoreSnapshot(const LevelSnapshot& snapshot) {
    restoreModelTree(snapshot.tree);
    restoreScore(snapshot.score);
    history = snapshot;
}

}
