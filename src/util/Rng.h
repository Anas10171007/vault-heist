#pragma once
#include "util/DynArray.h"

using namespace std;

namespace vh {

class Rng {
public:
    explicit Rng(unsigned int seed) {
        state = seed;
    }

    unsigned int next() {
        state = state * 1664525u + 1013904223u;
        return state;
    }

    int range(int lo, int hi) {
        if (lo > hi) {
            int temp = lo;
            lo = hi;
            hi = temp;
        }

        int numberOfValues = hi - lo + 1;
        int offset = static_cast<int>(next() % static_cast<unsigned int>(numberOfValues));
        return lo + offset;
    }

    template <typename T>
    void shuffle(DynArray<T>& values) {
        for (int i = values.size() - 1; i > 0; i--) {
            int randomIndex = range(0, i);
            T temp = values[i];
            values[i] = values[randomIndex];
            values[randomIndex] = temp;
        }
    }

private:
    unsigned int state;
};

}
