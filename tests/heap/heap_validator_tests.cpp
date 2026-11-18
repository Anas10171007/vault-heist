#include <iostream>
#include <string>
#include "model/heap/HeapValidator.h"
#include "model/heap/MaxHeap.h"

using namespace std;
using namespace vh;

static DynArray<int> validatorValues(int a, int b, int c, int d, int e, int f) {
    DynArray<int> values;
    values.push(a);
    values.push(b);
    values.push(c);
    values.push(d);
    values.push(e);
    values.push(f);
    return values;
}

int runHeapValidatorTests() {
    int failed = 0;
    auto check = [&](bool ok, const char* name) {
        cout << (ok ? "PASS " : "FAIL ") << name << endl;
        if (!ok) failed++;
    };

    string error;
    DynArray<int> valid = validatorValues(70, 60, 50, 20, 80, 90);
    check(HeapValidator::validate(valid, 4, 4, error) && error.empty(),
          "validator accepts a valid active heap and sorted region");

    DynArray<int> badLeft;
    badLeft.push(50);
    badLeft.push(70);
    badLeft.push(40);
    check(!HeapValidator::validate(badLeft, 3, 3, error) &&
          error.find("left child") != string::npos,
          "validator catches a larger left child");

    DynArray<int> badRight;
    badRight.push(50);
    badRight.push(40);
    badRight.push(70);
    check(!HeapValidator::validate(badRight, 3, 3, error) &&
          error.find("right child") != string::npos,
          "validator catches a larger right child");

    check(!HeapValidator::validate(valid, 9, 9, error) &&
          error.find("heap size") != string::npos,
          "validator catches a heap size outside the array");

    check(!HeapValidator::validate(valid, 4, 3, error) &&
          error.find("boundary") != string::npos,
          "validator catches mismatched heap and sorted boundaries");

    DynArray<int> unsortedTail = validatorValues(70, 60, 50, 20, 90, 80);
    check(!HeapValidator::validate(unsortedTail, 4, 4, error) &&
          error.find("not ascending") != string::npos,
          "validator catches an unsorted tail");

    DynArray<int> badBoundary = validatorValues(90, 70, 80, 20, 85, 95);
    check(!HeapValidator::validate(badBoundary, 4, 4, error) &&
          error.find("active heap maximum") != string::npos,
          "validator catches a sorted value smaller than the active maximum");

    DynArray<int> before = valid;
    HeapValidator::validate(valid, 4, 4, error);
    bool unchanged = true;
    for (int i = 0; i < valid.size(); i++) {
        if (valid[i] != before[i]) unchanged = false;
    }
    check(unchanged, "validator never changes the array");

    DynArray<int> raw;
    raw.push(3);
    raw.push(9);
    raw.push(2);
    raw.push(7);
    raw.push(5);
    raw.push(8);
    MaxHeap heap;
    heap.load(raw);
    heap.buildMaxHeap();
    check(HeapValidator::validate(heap, error), "validator accepts a built MaxHeap object");

    int maximum = 0;
    heap.extractMax(maximum);
    check(HeapValidator::validate(heap, error), "validator accepts a partially extracted heap");

    heap.heapSort();
    check(HeapValidator::validate(heap, error), "validator accepts a completed Heap Sort");

    return failed;
}
