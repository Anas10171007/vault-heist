#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include "model/Node.h"
#include "model/heap/MaxHeap.h"
#include "model/tree/AVLTree.h"
#include "model/tree/BST.h"
#include "util/DynArray.h"
#include "util/Rng.h"

using namespace std;
using namespace vh;

// Wall-clock helper. Call once before the work and once after, then subtract.
static long long microsNow() {
    return chrono::duration_cast<chrono::microseconds>(
               chrono::steady_clock::now().time_since_epoch())
        .count();
}

static DynArray<int> makeSequence(int n, bool randomOrder, unsigned seed) {
    DynArray<int> values;
    for (int i = 0; i < n; i++) values.push(i);
    if (randomOrder) {
        Rng rng(seed);
        rng.shuffle(values);
    }
    return values;
}

static DynArray<int> makeDuplicates(int n, unsigned seed) {
    DynArray<int> values;
    Rng rng(seed);
    for (int i = 0; i < n; i++) values.push(rng.range(0, n / 10 + 1));
    return values;
}

static DynArray<int> makeReverse(int n) {
    DynArray<int> values;
    for (int i = n - 1; i >= 0; i--) values.push(i);
    return values;
}

// Count comparisons by walking from the root the same way BST::search does.
static int countSearchComparisons(const BST& tree, int key) {
    const TreeNode* current = tree.root();
    int comparisons = 0;
    while (current != nullptr) {
        comparisons++;
        const BSTNode* node = static_cast<const BSTNode*>(current);
        if (key == node->key) break;
        if (key < node->key) current = current->left;
        else current = current->right;
    }
    return comparisons;
}

// Slow k-th on a BST: walk the inorder list until we reach position k.
static int bstKthByInorder(const BST& tree, int k) {
    BinaryTree::TraversalResult order = tree.inorder();
    if (k < 1 || k > order.ids.size()) return 0;
    const TreeNode* node = tree.findById(order.ids[k - 1]);
    if (node == nullptr) return 0;
    return node->value;
}

static void writeHeightCsv(const string& path, bool useAvl, bool randomOrder) {
    ofstream csv(path);
    csv << "n,height,microseconds,rotations\n";
    int sizes[3] = {100, 1000, 10000};
    for (int s = 0; s < 3; s++) {
        int n = sizes[s];
        DynArray<int> values = makeSequence(n, randomOrder, 0xA11u + n);
        long long start = microsNow();
        int height = 0;
        int rotations = 0;
        if (useAvl) {
            AVLTree tree(false);
            for (int i = 0; i < n; i++) tree.insert(values[i]);
            height = tree.height();
            rotations = tree.rotationCount();
        } else {
            BST tree(false);
            for (int i = 0; i < n; i++) tree.insert(values[i]);
            height = tree.height();
        }
        long long elapsed = microsNow() - start;
        csv << n << ',' << height << ',' << elapsed << ',' << rotations << '\n';
    }
}

static void writeSearchCsv(const string& path, bool useAvl, bool randomOrder) {
    ofstream csv(path);
    csv << "n,searches,total_comparisons,avg_comparisons,microseconds\n";
    int sizes[3] = {100, 1000, 10000};
    for (int s = 0; s < 3; s++) {
        int n = sizes[s];
        DynArray<int> values = makeSequence(n, randomOrder, 0xB22u + n);
        Rng rng(0xC33u + n);
        int searches = 10000;
        int totalComparisons = 0;
        long long elapsed = 0;

        if (useAvl) {
            AVLTree tree(false);
            for (int i = 0; i < n; i++) tree.insert(values[i]);
            long long start = microsNow();
            for (int i = 0; i < searches; i++) {
                int key = values[rng.range(0, n - 1)];
                totalComparisons += countSearchComparisons(tree, key);
                tree.search(key);
            }
            elapsed = microsNow() - start;
        } else {
            BST tree(false);
            for (int i = 0; i < n; i++) tree.insert(values[i]);
            long long start = microsNow();
            for (int i = 0; i < searches; i++) {
                int key = values[rng.range(0, n - 1)];
                totalComparisons += countSearchComparisons(tree, key);
                tree.search(key);
            }
            elapsed = microsNow() - start;
        }

        double average = (double)totalComparisons / (double)searches;
        csv << n << ',' << searches << ',' << totalComparisons << ',' << average << ','
            << elapsed << '\n';
    }
}

static void writeRotationCsv(const string& path) {
    ofstream csv(path);
    csv << "shape,n,insert_rotations,avg_insert_rotations,delete_rotations,"
           "avg_delete_rotations,max_rotations_per_delete\n";

    bool randomFlags[2] = {true, false};
    const char* names[2] = {"random", "sorted"};
    int sizes[3] = {100, 1000, 10000};

    for (int f = 0; f < 2; f++) {
        for (int s = 0; s < 3; s++) {
            int n = sizes[s];
            DynArray<int> values = makeSequence(n, randomFlags[f], 0xD44u + n + f);
            AVLTree tree(false);
            for (int i = 0; i < n; i++) tree.insert(values[i]);
            int insertRotations = tree.rotationCount();

            // Delete in a different order so deletes are not trivial.
            DynArray<int> deleteOrder = values;
            Rng rng(0xE55u + n + f);
            rng.shuffle(deleteOrder);

            int deleteRotations = 0;
            int maxPerDelete = 0;
            int before = tree.rotationCount();
            for (int i = 0; i < n; i++) {
                int rotBefore = tree.rotationCount();
                tree.deleteKey(deleteOrder[i]);
                int used = tree.rotationCount() - rotBefore;
                if (used > maxPerDelete) maxPerDelete = used;
            }
            deleteRotations = tree.rotationCount() - before;

            csv << names[f] << ',' << n << ',' << insertRotations << ','
                << (double)insertRotations / (double)n << ',' << deleteRotations << ','
                << (double)deleteRotations / (double)n << ',' << maxPerDelete << '\n';
        }
    }
}

static void writeOrderStatCsv(const string& path) {
    ofstream csv(path);
    csv << "n,queries,avl_kth_us,bst_inorder_us\n";
    int sizes[3] = {100, 1000, 10000};
    for (int s = 0; s < 3; s++) {
        int n = sizes[s];
        DynArray<int> values = makeSequence(n, true, 0xF66u + n);

        AVLTree avl(false);
        BST bst(false);
        for (int i = 0; i < n; i++) {
            avl.insert(values[i]);
            bst.insert(values[i]);
        }

        Rng rng(0x177u + n);
        int queries = 1000;
        long long avlStart = microsNow();
        for (int i = 0; i < queries; i++) {
            int k = rng.range(1, n);
            avl.kth(k);
        }
        long long avlElapsed = microsNow() - avlStart;

        long long bstStart = microsNow();
        for (int i = 0; i < queries; i++) {
            int k = rng.range(1, n);
            bstKthByInorder(bst, k);
        }
        long long bstElapsed = microsNow() - bstStart;

        csv << n << ',' << queries << ',' << avlElapsed << ',' << bstElapsed << '\n';
    }
}

static void writeHeapBuildCsv(const string& path) {
    ofstream csv(path);
    csv << "shape,n,microseconds,comparisons,swaps\n";
    const char* shapes[2] = {"random", "ordered"};
    bool randomFlags[2] = {true, false};
    int sizes[3] = {100, 1000, 10000};
    for (int f = 0; f < 2; f++) {
        for (int s = 0; s < 3; s++) {
            int n = sizes[s];
            DynArray<int> values = makeSequence(n, randomFlags[f], 0x288u + n + f);
            MaxHeap heap(false);
            heap.load(values);
            long long start = microsNow();
            heap.buildMaxHeap();
            long long elapsed = microsNow() - start;
            csv << shapes[f] << ',' << n << ',' << elapsed << ',' << heap.comparisons()
                << ',' << heap.swaps() << '\n';
        }
    }
}

static void writeHeapSortCsv(const string& path) {
    ofstream csv(path);
    csv << "shape,n,microseconds,comparisons,swaps\n";
    int sizes[3] = {100, 1000, 10000};
    for (int s = 0; s < 3; s++) {
        int n = sizes[s];

        DynArray<int> inputs[4];
        inputs[0] = makeSequence(n, true, 0x399u + n);
        inputs[1] = makeSequence(n, false, 0);
        inputs[2] = makeReverse(n);
        inputs[3] = makeDuplicates(n, 0x4AAu + n);
        const char* names[4] = {"random", "sorted", "reverse", "duplicates"};

        for (int kind = 0; kind < 4; kind++) {
            MaxHeap heap(false);
            heap.load(inputs[kind]);
            long long start = microsNow();
            heap.heapSort();
            long long elapsed = microsNow() - start;
            csv << names[kind] << ',' << n << ',' << elapsed << ',' << heap.comparisons()
                << ',' << heap.swaps() << '\n';
        }
    }
}

static void insertionSort(DynArray<int>& values, long long& comparisons, long long& moves) {
    comparisons = 0;
    moves = 0;
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
    comparisons = 0;
    swaps = 0;
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

static void writeSortCompareCsv(const string& path) {
    ofstream csv(path);
    csv << "n,heap_us,heap_comparisons,heap_swaps,heap_extra_space,"
           "insertion_us,insertion_comparisons,insertion_moves,insertion_extra_space,"
           "selection_us,selection_comparisons,selection_swaps,selection_extra_space\n";
    // Quadratic sorts stay on smaller n so the run finishes in reasonable time.
    int sizes[4] = {100, 500, 1000, 2000};
    for (int s = 0; s < 4; s++) {
        int n = sizes[s];
        DynArray<int> source = makeSequence(n, true, 0x5BBu + n);

        MaxHeap heap(false);
        heap.load(source);
        long long heapStart = microsNow();
        heap.heapSort();
        long long heapElapsed = microsNow() - heapStart;

        DynArray<int> insertionValues = source;
        long long insertComparisons = 0;
        long long insertMoves = 0;
        long long insertStart = microsNow();
        insertionSort(insertionValues, insertComparisons, insertMoves);
        long long insertElapsed = microsNow() - insertStart;

        DynArray<int> selectionValues = source;
        long long selectComparisons = 0;
        long long selectSwaps = 0;
        long long selectStart = microsNow();
        selectionSort(selectionValues, selectComparisons, selectSwaps);
        long long selectElapsed = microsNow() - selectStart;

        csv << n << ',' << heapElapsed << ',' << heap.comparisons() << ',' << heap.swaps()
            << ",0," << insertElapsed << ',' << insertComparisons << ',' << insertMoves
            << ",1," << selectElapsed << ',' << selectComparisons << ',' << selectSwaps
            << ",1\n";
    }
}

int main(int argc, char** argv) {
    string output = "bench/results";
    if (argc > 1) output = argv[1];

    cout << "Running Section 10 benchmarks into " << output << " ..." << endl;

    writeHeightCsv(output + "/height_bst_sorted.csv", false, false);
    writeHeightCsv(output + "/height_bst_random.csv", false, true);
    writeHeightCsv(output + "/height_avl_sorted.csv", true, false);
    writeHeightCsv(output + "/height_avl_random.csv", true, true);

    writeSearchCsv(output + "/search_bst_random.csv", false, true);
    writeSearchCsv(output + "/search_avl_random.csv", true, true);

    writeRotationCsv(output + "/avl_rotations.csv");
    writeOrderStatCsv(output + "/order_statistics.csv");

    writeHeapBuildCsv(output + "/heap_build.csv");
    writeHeapSortCsv(output + "/heap_sort.csv");
    writeSortCompareCsv(output + "/sort_comparison.csv");

    cout << "Done. Wrote height, search, rotation, order-stat, heap, and sort CSVs." << endl;
    return 0;
}
