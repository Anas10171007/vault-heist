#pragma once

#include "game/LevelDef.h"
#include "game/LevelPlay.h"

namespace vh {

// Returns nullptr for an unknown task. Ownership passes to the caller.
LevelPlay* createLevelPlay(const LevelDef& definition);

}
