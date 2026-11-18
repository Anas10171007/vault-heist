# B2 Heap validator

`HeapValidator` is read-only. It checks a raw heap state or a `MaxHeap` object
and returns `true` when valid. When invalid, the supplied string explains the
first problem found.

The checks follow project section 5.6:

- heap size and sorted boundary are inside the array;
- heap size equals the start of the sorted region;
- every active parent is at least as large as each existing child;
- the sorted region is ascending;
- the active maximum is no larger than the first sorted value.

In debug builds, `MaxHeap` runs the validator after build, insertion,
extraction, and Heap Sort. Unit tests also feed deliberately corrupted states
to the raw validator. Validation never repairs or changes the array.
