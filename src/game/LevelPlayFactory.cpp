#include "game/LevelPlayFactory.h"

#include "game/act1/Act1Level.h"

namespace vh {

LevelPlay* createLevelPlay(const LevelDef& definition) {
    const std::string& task = definition.task;
    if (task == "traversal_click" || task == "reconstruct" || task == "height_diameter" ||
        task == "lca_path" || task == "mirror") {
        return new Act1Level();
    }
    return nullptr;
}

}
