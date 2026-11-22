#include "game/LevelDataBuilder.h"

#include "model/heap/MaxHeap.h"
#include "util/Rng.h"

namespace vh {

static void fillRandomPool(Rng& rng, const LevelData& data, DynArray<int>& output,
                           bool allowDuplicates) {
    output.clear();
    if (data.size <= 0) return;

    if (allowDuplicates) {
        // Small value pool so duplicates show up often.
        int lo = data.minValue;
        int hi = data.maxValue;
        int span = hi - lo + 1;
        if (span > 8) hi = lo + 7;
        for (int i = 0; i < data.size; i++) {
            output.push(rng.range(lo, hi));
        }
        return;
    }

    DynArray<int> pool;
    for (int value = data.minValue; value <= data.maxValue && value < data.minValue + 10000;
         value++) {
        pool.push(value);
    }
    if (pool.size() > 0) rng.shuffle(pool);
    for (int i = 0; i < data.size; i++) {
        if (i < pool.size()) output.push(pool[i]);
        else output.push(rng.range(data.minValue, data.maxValue));
    }
}

static void heapifyInPlace(DynArray<int>& values) {
    MaxHeap heap(false);
    heap.load(values);
    heap.buildMaxHeap();
    values.clear();
    for (int i = 0; i < heap.size(); i++) values.push(heap.at(i));
}

static void disturbHeap(DynArray<int>& values, int disturbCount, Rng& rng) {
    if (values.size() < 2) return;
    int swaps = disturbCount;
    if (swaps < 1) swaps = 1;
    if (swaps > values.size()) swaps = values.size();
    for (int i = 0; i < swaps; i++) {
        int child = rng.range(1, values.size() - 1);
        int parent = (child - 1) / 2;
        int temporary = values[child];
        values[child] = values[parent];
        values[parent] = temporary;
    }
}

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
    if (span < 1) span = 1;
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
    if (data.shape == "duplicates") {
        fillRandomPool(rng, data, output, true);
        return;
    }

    // random, heap, disturbed — start from a shuffled distinct-ish pool
    fillRandomPool(rng, data, output, false);

    if (data.shape == "heap" || data.shape == "disturbed") {
        heapifyInPlace(output);
    }
    if (data.shape == "disturbed") {
        disturbHeap(output, data.disturb, rng);
    }
    // Pure random can accidentally already be a Max Heap — nudge it so build
    // levels still need real swaps.
    if (data.shape == "random" && output.size() >= 2) {
        MaxHeap probe(false);
        probe.load(output);
        if (probe.isMaxHeap()) disturbHeap(output, 2, rng);
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
