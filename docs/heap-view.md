# B3 Heap view

`HeapView` is a read-only visual replay of heap events. It keeps one visual
array, and both the tree and array drawings use that same state.

- Tree nodes use array index arithmetic and only show the active heap.
- The array shows every value and derives the sorted boundary from `heapSize`.
- `HEAP_SWAP` moves the same two indexes in both drawings.
- `HEAP_SIZE_CHANGE` moves the active/sorted boundary.
- The root carries a `MAX` badge.
- Invalid parent-child pairs carry the `BAD` state.
- Long arrays wrap according to the available width.

The starting array is kept only so `AnimationController` can rebuild the view
for reset, backward stepping and replay. It is not another heap model and it
never changes `MaxHeap`.

Run `heapviewdemo`. Use Space to play or pause, Left and Right to step, R to
reset, and Escape to close.
