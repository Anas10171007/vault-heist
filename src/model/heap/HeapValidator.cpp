#include "model/heap/HeapValidator.h"
#include "model/heap/MaxHeap.h"

namespace vh {

bool HeapValidator::validate(const MaxHeap& heap, string& error) {
    return validate(heap.values, heap.heapSize, heap.heapSize, error);
}

bool HeapValidator::validate(const DynArray<int>& values, int heapSize,
                             int sortedFrom, string& error) {
    error.clear();
    int totalSize = values.size();

    if (heapSize < 0 || heapSize > totalSize) {
        error = "heap size is outside the array";
        return false;
    }
    if (sortedFrom < 0 || sortedFrom > totalSize) {
        error = "sorted region starts outside the array";
        return false;
    }
    if (heapSize != sortedFrom) {
        error = "heap size and sorted region boundary do not match";
        return false;
    }

    for (int parent = 0; parent < heapSize / 2; parent++) {
        int left = parent * 2 + 1;
        int right = parent * 2 + 2;
        if (left < heapSize && values[parent] < values[left]) {
            error = "parent at index " + to_string(parent) +
                    " is smaller than left child at index " + to_string(left);
            return false;
        }
        if (right < heapSize && values[parent] < values[right]) {
            error = "parent at index " + to_string(parent) +
                    " is smaller than right child at index " + to_string(right);
            return false;
        }
    }

    for (int index = sortedFrom + 1; index < totalSize; index++) {
        if (values[index - 1] > values[index]) {
            error = "sorted region is not ascending at indices " +
                    to_string(index - 1) + " and " + to_string(index);
            return false;
        }
    }

    if (heapSize > 0 && sortedFrom < totalSize && values[0] > values[sortedFrom]) {
        error = "active heap maximum is larger than the first sorted value";
        return false;
    }

    return true;
}

}
