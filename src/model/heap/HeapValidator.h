#pragma once
#include <string>
#include "util/DynArray.h"

using namespace std;

namespace vh {

class MaxHeap;

class HeapValidator {
public:
    static bool validate(const MaxHeap& heap, string& error);
    static bool validate(const DynArray<int>& values, int heapSize,
                         int sortedFrom, string& error);
};

}
