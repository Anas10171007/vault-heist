#pragma once
#include <string>
#include "game/JsonReader.h"
#include "game/LevelDef.h"

using namespace std;

namespace vh {

class LevelManager {
public:
    bool loadAll(const string& levelsFolder);
    bool loadFile(const string& path, int expectedAct);
    const LevelDef* load(const string& id) const;
    const LevelDef& at(int index) const;
    int size() const;
    void clear();
    const string& error() const;

private:
    DynArray<LevelDef> levels;
    string errorMessage;

    bool parseRoot(JsonReader& reader, int expectedAct, DynArray<LevelDef>& parsed);
    bool parseLevels(JsonReader& reader, DynArray<LevelDef>& parsed);
    bool parseLevel(JsonReader& reader, LevelDef& level);
    bool parseData(JsonReader& reader, LevelData& data);
    bool parseRules(JsonReader& reader, LevelRules& rules);
    bool parseIntArray(JsonReader& reader, DynArray<int>& values);
    bool validateLevel(const LevelDef& level, int act);
    bool taskMatchesStructure(const string& task, const string& structure) const;
    bool missing(const LevelDef& level, const string& field);
};

}
