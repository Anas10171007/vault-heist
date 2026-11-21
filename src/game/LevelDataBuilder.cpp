#include "game/LevelDataBuilder.h"

#include "util/Rng.h"

namespace vh {

void LevelDataBuilder::buildValues(const LevelDef& definition, DynArray<int>& output) {
    output.clear();
    const LevelData& data = definition.data;
    if (data.source == "explicit") {
        for (int i = 0; i < data.values.size(); i++) {
            output.push(data.values[i]);
        }
        return;
    }

    int span = data.maxValue - data.minValue + 1;
    if (span < 1) {
        span = 1;
    }
    Rng rng(static_cast<unsigned int>(definition.seed));

    if (data.shape == "sorted") {
        for (int i = 0; i < data.size; i++) {
            output.push(data.minValue + i % span);
        }
        return;
    }
    if (data.shape == "reverse") {
        for (int i = 0; i < data.size; i++) {
            output.push(data.maxValue - i % span);
        }
        return;
    }

    DynArray<int> pool;
    for (int value = data.minValue; value <= data.maxValue && value < data.minValue + 10000; value++) {
        pool.push(value);
    }
    if (pool.size() > 0) {
        rng.shuffle(pool);
    }
    for (int i = 0; i < data.size; i++) {
        if (i < pool.size()) {
            output.push(pool[i]);
        } else {
            output.push(rng.range(data.minValue, data.maxValue));
        }
    }
}

bool LevelDataBuilder::isDistinct(const DynArray<int>& values) {
    for (int i = 0; i < values.size(); i++) {
        for (int j = i + 1; j < values.size(); j++) {
            if (values[i] == values[j]) {
                return false;
            }
        }
    }
    return true;
}

}
