#include "game/LevelManager.h"
#include <fstream>

namespace vh {

bool LevelManager::loadAll(const string& levelsFolder) {
    clear();
    for (int act = 1; act <= 4; act++) {
        string path = levelsFolder + "/act" + to_string(act) + ".json";
        if (!loadFile(path, act)) {
            levels.clear();
            return false;
        }
    }
    return true;
}

bool LevelManager::loadFile(const string& path, int expectedAct) {
    errorMessage.clear();
    ifstream file(path);
    if (!file.is_open()) {
        errorMessage = "could not open " + path;
        return false;
    }

    string text;
    char character = 0;
    while (file.get(character)) text += character;

    JsonReader reader(text);
    DynArray<LevelDef> parsed;
    if (!parseRoot(reader, expectedAct, parsed)) {
        if (errorMessage.empty()) errorMessage = reader.error();
        return false;
    }

    for (int i = 0; i < parsed.size(); i++) {
        if (load(parsed[i].id) != nullptr) {
            errorMessage = "duplicate level id " + parsed[i].id;
            return false;
        }
        for (int j = 0; j < i; j++) {
            if (parsed[j].id == parsed[i].id) {
                errorMessage = "duplicate level id " + parsed[i].id;
                return false;
            }
        }
    }

    for (int i = 0; i < parsed.size(); i++) levels.push(parsed[i]);
    errorMessage.clear();
    return true;
}

bool LevelManager::parseRoot(JsonReader& reader, int expectedAct, DynArray<LevelDef>& parsed) {
    if (!reader.consume('{')) return false;
    bool hasVersion = false;
    bool hasAct = false;
    bool hasLevels = false;
    int version = 0;
    int act = 0;

    if (!reader.nextIs('}')) {
        while (true) {
            string key;
            if (!reader.readString(key) || !reader.consume(':')) return false;
            if (key == "version") {
                if (!reader.readInt(version)) return false;
                hasVersion = true;
            } else if (key == "act") {
                if (!reader.readInt(act)) return false;
                hasAct = true;
            } else if (key == "levels") {
                if (!parseLevels(reader, parsed)) return false;
                hasLevels = true;
            } else if (!reader.skipValue()) {
                return false;
            }

            if (reader.nextIs('}')) break;
            if (!reader.consume(',')) return false;
        }
    }
    if (!reader.consume('}') || !reader.finished()) return false;

    if (!hasVersion) {
        errorMessage = "level file missing field version";
        return false;
    }
    if (!hasAct) {
        errorMessage = "level file missing field act";
        return false;
    }
    if (!hasLevels) {
        errorMessage = "level file missing field levels";
        return false;
    }
    if (version != 1) {
        errorMessage = "unsupported level file version " + to_string(version);
        return false;
    }
    if (act != expectedAct) {
        errorMessage = "expected act " + to_string(expectedAct) + " but file says act " + to_string(act);
        return false;
    }

    for (int i = 0; i < parsed.size(); i++) {
        parsed[i].act = act;
        if (!validateLevel(parsed[i], act)) return false;
    }
    return true;
}

bool LevelManager::parseLevels(JsonReader& reader, DynArray<LevelDef>& parsed) {
    if (!reader.consume('[')) return false;
    if (reader.nextIs(']')) return reader.consume(']');

    while (true) {
        LevelDef level;
        if (!parseLevel(reader, level)) return false;
        parsed.push(level);
        if (reader.nextIs(']')) return reader.consume(']');
        if (!reader.consume(',')) return false;
    }
}

bool LevelManager::parseLevel(JsonReader& reader, LevelDef& level) {
    if (!reader.consume('{')) return false;
    bool hasId = false;
    bool hasTitle = false;
    bool hasMission = false;
    bool hasStructure = false;
    bool hasTask = false;
    bool hasSeed = false;
    bool hasData = false;
    bool hasRules = false;

    if (!reader.nextIs('}')) {
        while (true) {
            string key;
            if (!reader.readString(key) || !reader.consume(':')) return false;
            if (key == "id") {
                if (!reader.readString(level.id)) return false;
                hasId = true;
            } else if (key == "title") {
                if (!reader.readString(level.title)) return false;
                hasTitle = true;
            } else if (key == "mission") {
                if (!reader.readString(level.mission)) return false;
                hasMission = true;
            } else if (key == "structure") {
                if (!reader.readString(level.structure)) return false;
                hasStructure = true;
            } else if (key == "task") {
                if (!reader.readString(level.task)) return false;
                hasTask = true;
            } else if (key == "seed") {
                if (!reader.readInt(level.seed)) return false;
                hasSeed = true;
            } else if (key == "data") {
                if (!parseData(reader, level.data)) return false;
                hasData = true;
            } else if (key == "rules") {
                if (!parseRules(reader, level.rules)) return false;
                hasRules = true;
            } else if (!reader.skipValue()) {
                return false;
            }

            if (reader.nextIs('}')) break;
            if (!reader.consume(',')) return false;
        }
    }
    if (!reader.consume('}')) return false;

    string name = hasId ? level.id : "unknown level";
    if (!hasId) return missing(level, "id");
    if (!hasTitle) return missing(level, "title");
    if (!hasMission) return missing(level, "mission");
    if (!hasStructure) return missing(level, "structure");
    if (!hasTask) return missing(level, "task");
    if (!hasSeed) return missing(level, "seed");
    if (!hasData) return missing(level, "data");
    if (!hasRules) return missing(level, "rules");
    (void)name;
    return true;
}

bool LevelManager::parseData(JsonReader& reader, LevelData& data) {
    if (!reader.consume('{')) return false;
    if (!reader.nextIs('}')) {
        while (true) {
            string key;
            if (!reader.readString(key) || !reader.consume(':')) return false;
            if (key == "source") {
                if (!reader.readString(data.source)) return false;
            } else if (key == "tree") {
                if (!reader.readString(data.tree)) return false;
                data.hasTree = true;
            } else if (key == "values") {
                if (!parseIntArray(reader, data.values)) return false;
                data.hasValues = true;
            } else if (key == "shape") {
                if (!reader.readString(data.shape)) return false;
                data.hasShape = true;
            } else if (key == "size") {
                if (!reader.readInt(data.size)) return false;
            } else if (key == "min") {
                if (!reader.readInt(data.minValue)) return false;
            } else if (key == "max") {
                if (!reader.readInt(data.maxValue)) return false;
            } else if (key == "disturb") {
                if (!reader.readInt(data.disturb)) return false;
            } else if (!reader.skipValue()) {
                return false;
            }
            if (reader.nextIs('}')) break;
            if (!reader.consume(',')) return false;
        }
    }
    if (!reader.consume('}')) return false;
    return true;
}

bool LevelManager::parseRules(JsonReader& reader, LevelRules& rules) {
    if (!reader.consume('{')) return false;
    if (!reader.nextIs('}')) {
        while (true) {
            string key;
            if (!reader.readString(key) || !reader.consume(':')) return false;
            if (key == "traversal") {
                if (!reader.readString(rules.traversal)) return false;
            } else if (key == "from") {
                if (!reader.readString(rules.from)) return false;
            } else if (key == "rotation") {
                if (!reader.readString(rules.rotation)) return false;
            } else if (key == "a") {
                if (!reader.readInt(rules.a)) return false;
            } else if (key == "b") {
                if (!reader.readInt(rules.b)) return false;
            } else if (key == "target") {
                if (!reader.readInt(rules.target)) return false;
            } else if (key == "lo") {
                if (!reader.readInt(rules.lo)) return false;
            } else if (key == "hi") {
                if (!reader.readInt(rules.hi)) return false;
            } else if (key == "k") {
                if (!reader.readInt(rules.k)) return false;
            } else if (key == "count") {
                if (!reader.readInt(rules.count)) return false;
            } else if (key == "inserts") {
                if (!parseIntArray(reader, rules.inserts)) return false;
            } else if (!reader.skipValue()) {
                return false;
            }
            if (reader.nextIs('}')) break;
            if (!reader.consume(',')) return false;
        }
    }
    return reader.consume('}');
}

bool LevelManager::parseIntArray(JsonReader& reader, DynArray<int>& values) {
    if (!reader.consume('[')) return false;
    if (reader.nextIs(']')) return reader.consume(']');
    while (true) {
        int value = 0;
        if (!reader.readInt(value)) return false;
        values.push(value);
        if (reader.nextIs(']')) return reader.consume(']');
        if (!reader.consume(',')) return false;
    }
}

bool LevelManager::missing(const LevelDef& level, const string& field) {
    string name = level.id.empty() ? "unknown level" : "level " + level.id;
    errorMessage = name + " missing field " + field;
    return false;
}

bool LevelManager::validateLevel(const LevelDef& level, int act) {
    if (level.id.size() < 3 || level.id[0] != static_cast<char>('0' + act) || level.id[1] != '.') {
        errorMessage = "level " + level.id + " id does not match act " + to_string(act);
        return false;
    }
    if (!taskMatchesStructure(level.task, level.structure)) {
        errorMessage = "level " + level.id + " task " + level.task + " does not match " + level.structure;
        return false;
    }
    if (level.data.source.empty()) {
        errorMessage = "level " + level.id + " missing field data.source";
        return false;
    }
    if (level.data.source == "explicit") {
        if (level.data.hasTree == level.data.hasValues) {
            errorMessage = "level " + level.id + " explicit data needs exactly one of tree or values";
            return false;
        }
        if (level.structure == "binary" && !level.data.hasTree) {
            errorMessage = "level " + level.id + " binary data needs a tree";
            return false;
        }
        if (level.structure == "maxheap" && !level.data.hasValues) {
            errorMessage = "level " + level.id + " maxheap data needs values";
            return false;
        }
    } else if (level.data.source == "generate") {
        if (!level.data.hasShape || level.data.size <= 0) {
            errorMessage = "level " + level.id + " generated data needs shape and positive size";
            return false;
        }
        if (level.data.minValue > level.data.maxValue) {
            errorMessage = "level " + level.id + " has min greater than max";
            return false;
        }
    } else {
        errorMessage = "level " + level.id + " has unknown data source " + level.data.source;
        return false;
    }
    return true;
}

bool LevelManager::taskMatchesStructure(const string& task, const string& structure) const {
    if (structure == "binary") {
        return task == "traversal_click" || task == "reconstruct" || task == "height_diameter" ||
               task == "lca_path" || task == "mirror";
    }
    if (structure == "bst") {
        return task == "bst_search" || task == "bst_insert" || task == "bst_delete" ||
               task == "bst_range" || task == "bst_boss";
    }
    if (structure == "avl") {
        return task == "avl_badges" || task == "avl_rotate_single" || task == "avl_rotate_double" ||
               task == "avl_auto_predict" || task == "avl_kth";
    }
    if (structure == "maxheap") {
        return task == "heap_build" || task == "heap_repair" || task == "heap_extract" ||
               task == "heap_race" || task == "heap_sort";
    }
    return false;
}

const LevelDef* LevelManager::load(const string& id) const {
    for (int i = 0; i < levels.size(); i++) {
        if (levels[i].id == id) return &levels[i];
    }
    return nullptr;
}

const LevelDef& LevelManager::at(int index) const {
    return levels[index];
}

int LevelManager::size() const {
    return levels.size();
}

void LevelManager::clear() {
    levels.clear();
    errorMessage.clear();
}

const string& LevelManager::error() const {
    return errorMessage;
}

}
