# Benchmark notes

Run from the repository root after building:

```powershell
cmake --build build --target bench -- -j1
.\build\bin\bench.exe
```

Results go to `bench/results/`. Pass another folder as the first argument if you want.

Recording is off for every structure so event logs do not distort timings.
Seeds are fixed, so the same machine should reproduce the same comparison and
swap counts. Wall-clock microseconds still vary a little between runs.

## What each CSV measures (brief Section 10)

| File | Experiment |
| --- | --- |
| `height_bst_*.csv`, `height_avl_*.csv` | Final height after inserting n = 100, 1000, 10000 keys, sorted vs random |
| `search_bst_random.csv`, `search_avl_random.csv` | 10,000 searches: average comparisons and time |
| `avl_rotations.csv` | Insert and delete rotations, random vs sorted; average and max per delete |
| `order_statistics.csv` | 1,000 queries: AVL `kth` vs BST full inorder scan |
| `heap_build.csv` | Build Max Heap, random and ordered inputs |
| `heap_sort.csv` | Heap Sort on random, sorted, reverse, and duplicate-heavy inputs |
| `sort_comparison.csv` | Heap Sort vs hand-written insertion and selection sort |

## How to read the numbers

- **Height.** Sorted BST grows into a long chain (height near n). Random BST is
  much shorter on average. AVL stays near log n for both shapes because it
  rotates while inserting.
- **Search.** More height means more comparisons. AVL search cost stays close to
  log n; skewed BST search cost grows with n.
- **Rotations.** Sorted inserts force many AVL rotations. Random inserts need
  fewer. Deletes can spike; the max-per-delete column shows the worst single
  delete in that run.
- **Order statistics.** AVL `kth` uses subtree sizes (about log n). The BST
  baseline rebuilds / walks an inorder list for every query (about n), so it
  slows down faster as n grows.
- **Heap build.** Bottom-up build is O(n). Comparisons and swaps should grow
  roughly linearly with n, not like n log n.
- **Heap Sort.** O(n log n). Sorted, reverse, and duplicate inputs change the
  constant factors but not the overall growth.
- **Sort comparison.** Insertion and selection are O(n^2). Heap Sort wins on
  larger n. Extra space: heap sort is in-place (0); the quadratic sorts use a
  few temporaries (counted as 1 in the CSV).

No `std::sort` is used anywhere in this suite.
