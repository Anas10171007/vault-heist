#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include "model/heap/MaxHeap.h"
#include "model/tree/AVLTree.h"
#include "model/tree/BST.h"
#include "util/DynArray.h"
#include "util/Rng.h"

using namespace std;
using namespace vh;
using Clock = chrono::steady_clock;

static DynArray<int> sequence(int n, bool randomOrder) {
    DynArray<int> values;
    for (int i = 0; i < n; i++) values.push(i);
    if (randomOrder) {
        Rng rng(0xC0FFEEu + static_cast<unsigned>(n));
        rng.shuffle(values);
    }
    return values;
}

template <typename Function>
static long long micros(Function operation) {
    auto start = Clock::now();
    operation();
    return chrono::duration_cast<chrono::microseconds>(Clock::now() - start).count();
}

static void treeBench(const string& path, bool avl, bool randomOrder) {
    ofstream csv(path);
    csv << "n,microseconds,height,rotations\n";
    const int sizes[] = {100, 500, 1000, 2500, 5000};
    for (int n : sizes) {
        DynArray<int> values = sequence(n, randomOrder);
        int height = 0;
        int rotations = 0;
        long long elapsed = 0;
        if (avl) {
            AVLTree tree(false);
            elapsed = micros([&] { for (int i = 0; i < n; i++) tree.insert(values[i]); });
            height = tree.height();
            rotations = tree.rotationCount();
        } else {
            BST tree(false);
            elapsed = micros([&] { for (int i = 0; i < n; i++) tree.insert(values[i]); });
            height = tree.height();
        }
        csv << n << ',' << elapsed << ',' << height << ',' << rotations << '\n';
    }
}

static void heapBench(const string& path, bool sort) {
    ofstream csv(path);
    csv << "n,microseconds,comparisons,swaps\n";
    const int sizes[] = {100, 500, 1000, 2500, 5000, 10000};
    for (int n : sizes) {
        DynArray<int> values = sequence(n, true);
        MaxHeap heap(false);
        heap.load(values);
        long long elapsed = micros([&] { if (sort) heap.heapSort(); else heap.buildMaxHeap(); });
        csv << n << ',' << elapsed << ',' << heap.comparisons() << ',' << heap.swaps() << '\n';
    }
}

static void insertionSort(DynArray<int>& values, long long& comparisons, long long& moves) {
    for (int i = 1; i < values.size(); i++) {
        int value = values[i];
        int j = i - 1;
        while (j >= 0) {
            comparisons++;
            if (values[j] <= value) break;
            values[j + 1] = values[j];
            moves++;
            j--;
        }
        values[j + 1] = value;
        moves++;
    }
}

static void selectionSort(DynArray<int>& values, long long& comparisons, long long& swaps) {
    for (int i = 0; i < values.size(); i++) {
        int smallest = i;
        for (int j = i + 1; j < values.size(); j++) {
            comparisons++;
            if (values[j] < values[smallest]) smallest = j;
        }
        if (smallest != i) {
            int value = values[i];
            values[i] = values[smallest];
            values[smallest] = value;
            swaps++;
        }
    }
}

static void quadraticBench(const string& path) {
    ofstream csv(path);
    csv << "n,insertion_us,insertion_comparisons,selection_us,selection_comparisons\n";
    const int sizes[] = {100, 250, 500, 1000, 2000};
    for (int n : sizes) {
        DynArray<int> source = sequence(n, true);
        DynArray<int> insertionValues = source;
        DynArray<int> selectionValues = source;
        long long insertComparisons = 0, insertMoves = 0;
        long long selectComparisons = 0, selectSwaps = 0;
        long long insertionTime = micros([&] { insertionSort(insertionValues, insertComparisons, insertMoves); });
        long long selectionTime = micros([&] { selectionSort(selectionValues, selectComparisons, selectSwaps); });
        csv << n << ',' << insertionTime << ',' << insertComparisons << ','
            << selectionTime << ',' << selectComparisons << '\n';
    }
}

int main(int argc, char** argv) {
    string output = argc > 1 ? argv[1] : "bench/results";
    filesystem::create_directories(output);
    treeBench(output + "/bst_sorted_insert.csv", false, false);
    treeBench(output + "/bst_random_insert.csv", false, true);
    treeBench(output + "/avl_sorted_insert.csv", true, false);
    treeBench(output + "/avl_random_insert.csv", true, true);
    heapBench(output + "/heap_build.csv", false);
    heapBench(output + "/heap_sort.csv", true);
    quadraticBench(output + "/insertion_vs_selection.csv");
    cout << "wrote 7 benchmark CSV files to " << output << endl;
    return 0;
}
