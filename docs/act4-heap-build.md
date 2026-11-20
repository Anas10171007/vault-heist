# B4.1 Activate the Vault

Level 4.1 is the first playable Max Heap campaign level. The player selects a
parent and child in either the tree or array view and performs the swaps needed
to build a Max Heap.

`HeapLevelScreen` handles clicks and drawing. `HeapBuildLevel` checks the game
rules and scoring. `MaxHeap::playerSwap` changes the array and returns a
`HEAP_SWAP` event with the `PLAYER` reason. `HeapView` replays that event in the
tree and array at the same time.

A swap is accepted when the selected indexes are directly connected, the child
is larger than the parent, and it is the parent's largest child. An invalid
swap changes nothing and adds one alarm penalty. The level finishes when
`MaxHeap::isMaxHeap()` returns true.

The optimal step count is calculated by building a temporary non-recording
Max Heap and reading its swap count. It is not hard-coded into the screen.
