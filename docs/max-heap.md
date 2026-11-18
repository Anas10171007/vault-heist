# B1 Max Heap model

`MaxHeap` stores values in the custom `DynArray<int>`. `heapSize` is the number
of cells that still belong to the active heap. Cells from `heapSize` through
`values.size() - 1` are the sorted region produced by extraction or Heap Sort.

The zero-based index formulas are:

```text
parent = (index - 1) / 2
left   = 2 * index + 1
right  = 2 * index + 2
```

Insertion adds a value at the end and repairs upward. Building, extraction and
sorting repair downward by comparing a parent with both children and swapping
with the larger child. Equal values do not swap because duplicates are allowed.

Every comparison and swap is counted for later benchmarks. Heap changes emit
the shared events in `docs/event-payloads.md`. In particular,
`HEAP_SIZE_CHANGE` stores its old and new sizes in `oldValue` and `newValue`.
Recording can be disabled for benchmarks without changing the heap operations.
Heap insertion reuses the shared `INSERT` event: `idA` is the new array index,
`side` is `NONE`, and `key` stores the inserted value.

Insertion is rejected while a sorted region exists. The caller must load or
rebuild a normal active heap first, so extracted data is never silently lost.
