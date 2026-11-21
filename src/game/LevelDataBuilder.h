#pragma once

#include "game/LevelDef.h"
#include "util/DynArray.h"

namespace vh {

// Deterministic level data from the seed, exactly as described in
// docs/level-format.md. No global random state is touched.
class LevelDataBuilder {
public:
    static void buildValues(const LevelDef& definition, DynArray<int>& output);
    static bool isDistinct(const DynArray<int>& values);
};

}
