#include "game/LevelPlayFactory.h"

#include "game/act1/Act1Level.h"
#include "game/act2/Act2Level.h"

namespace vh {

LevelPlay* createLevelPlay(const LevelDef& definition) {
    const std::string& task = definition.task;
    if (task == "traversal_click" || task == "reconstruct" || task == "height_diameter" ||
        task == "lca_path" || task == "mirror") {
        return new Act1Level();
    }
    if (task == "bst_search" || task == "bst_insert" || task == "bst_delete" ||
        task == "bst_range" || task == "bst_boss") {
        return new Act2Level();
    }
    return nullptr;
}

}
