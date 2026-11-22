# Vault Heist test report

Generated from the real `tests.exe` output on 21 November 2026.

## How to run

From the repository root:

```powershell
cmake -S . -B build
cmake --build build -j 4
.\build\bin\tests.exe
```

Each check prints one `PASS <case>` or `FAIL <case>` line. The run ends with
`all passed` when every check succeeds.

## Totals

| Scope | Checks | Passed | Failed |
| --- | ---: | ---: | ---: |
| Tree suites (BinaryTree, BST, AVL, validator, layout) | 415 | 415 | 0 |
| Heap suites (model, validator, view, Act 4 levels) | 73 | 73 | 0 |
| Whole executable, all suites | 621 | 621 | 0 |

The non-tree suites (utilities, animation, levels, score, heap model, heap
validator, heap view, all five Act 4 levels, and heap snapshot undo) are included in the whole-executable
row.

The current Act 4 coverage includes empty, singleton, duplicate, negative,
already-valid, reverse-sorted and random builds, repeated-extraction, full-sort
against a hand-written reference sort, serialization round-trip, invalid
snapshot, extract-then-heapify gameplay, level scoring, and undo cases. The
final line of the captured run is `all passed`.

## Tree suites

### BinaryTree - 81 checks, 81 passed, 0 failed

- `empty tree traversals are empty`
- `empty tree measures zero`
- `empty tree serializes as a null marker`
- `empty tree is symmetric`
- `empty tree equals itself`
- `single node deserializes`
- `single node measures one`
- `single node round-trips`
- `single node appears once in every traversal`
- `traversal emits one VISIT per node`
- `seven-node tree deserializes`
- `seven-node tree measures height and diameter`
- `preorder returns the right values`
- `inorder returns the right values`
- `postorder returns the right values`
- `level order returns the right values`
- `recursive and iterative preorder agree`
- `recursive and iterative inorder agree`
- `recursive and iterative postorder agree`
- `recursive and iterative level order agree`
- `preorder VISIT events follow the traversal`
- `inorder VISIT events follow the traversal`
- `postorder VISIT events follow the traversal`
- `level order VISIT events follow the traversal`
- `parent pointers are consistent after deserialization`
- `skewed tree measures height and diameter`
- `skewed preorder recursive equals iterative`
- `skewed inorder recursive equals iterative`
- `skewed postorder recursive equals iterative`
- `skewed level order recursive equals iterative`
- `preorder plus inorder rebuilds the tree`
- `rebuilt tree serializes identically`
- `rebuild emits one INSERT per node`
- `rebuild ends with DONE`
- `postorder plus inorder rebuilds the tree`
- `parent pointers are consistent after reconstruction`
- `diameter finds a path that does not cross the root`
- `size mismatch is rejected with a message`
- `failed rebuild leaves the tree unchanged`
- `duplicate reconstruction input is rejected`
- `inconsistent traversals are rejected`
- `empty reconstruction clears the tree`
- `lca of siblings is their parent`
- `lca across subtrees is the root`
- `lca of the root and a descendant is the root`
- `lca with a missing id returns null`
- `pathTo returns the root-to-node path`
- `pathTo the root has one node`
- `pathTo a missing id is empty`
- `mirror returns an event log`
- `mirror swaps children recursively`
- `mirror reverses the inorder sequence`
- `mirror preserves measurements`
- `mirroring twice restores the tree`
- `node ids survive mirror`
- `identical trees compare equal`
- `trees with different values differ`
- `trees with different shapes differ`
- `a symmetric tree is detected`
- `an asymmetric tree is rejected`
- `the seven-node sample is not symmetric`
- `serialize and deserialize round-trip`
- `round-trip preserves structure and values`
- `a single null marker clears the tree`
- `whitespace between tokens is accepted`
- `out-of-range integers are rejected`
- `negative values round-trip`
- `recording off still returns the traversal`
- `recording on emits events again`
- `recording off keeps deserialization working without events`
- `recording off keeps reconstruction working without events`
- `recording off keeps mirror working without events`
- `node ids are not reused after clear`
- `incomplete serialization is rejected`
- `failed deserialization leaves the tree unchanged`
- `invalid token is rejected`
- `trailing tokens are rejected`
- `empty text is rejected`
- `every failed deserialization leaves the tree unchanged`
- `move construction transfers ownership`
- `move assignment transfers ownership`

### BST - 136 checks, 136 passed, 0 failed

- `empty BST has no min or max`
- `empty BST measures zero`
- `checker rejects a tree with broken ordering`
- `checker rejects a broken parent link`
- `checker rejects a key/value mismatch`
- `insert into an empty tree succeeds`
- `first insert becomes the root`
- `empty insert emits one INSERT`
- `root INSERT has parent 0 and side NONE`
- `insert ends with DONE`
- `checker accepts the root`
- `ascending inserts keep the BST valid after every insert`
- `ascending inserts produce sorted inorder`
- `ascending inserts build a right chain`
- `descending inserts keep the BST valid after every insert`
- `descending inserts produce sorted inorder`
- `descending inserts build a left chain`
- `mixed inserts keep the BST valid after every insert`
- `mixed inserts produce sorted inorder`
- `mixed inserts measure height and size`
- `200 seeded random inserts keep the BST valid after every insert`
- `200 random inserts produce 200 nodes`
- `200 random inserts produce the sorted inorder`
- `insert compares along the path`
- `COMPARE extra reports the searched key as smaller`
- `insert emits INSERT immediately before DONE`
- `duplicate insert is rejected with the exact message`
- `duplicate insert leaves the tree unchanged`
- `duplicate insert emits no INSERT`
- `duplicate insert ends with DONE`
- `duplicate search ends on an equal comparison`
- `search finds an existing key`
- `search emits one COMPARE per path node`
- `search ends with DONE`
- `search reports a missing key`
- `missing search compares every node on the path`
- `search on an empty tree compares nothing`
- `leaf delete removes the key`
- `leaf UNLINK carries node, parent, side, key and no child`
- `delete ends with DONE`
- `checker accepts the tree after a leaf delete`
- `one-left-child UNLINK carries the bypassed child id`
- `bypassed left child is relinked to the parent`
- `checker accepts the tree after a one-left-child delete`
- `one-right-child UNLINK carries the bypassed child id`
- `bypassed right child is relinked to the parent`
- `checker accepts the tree after a one-right-child delete`
- `two-child delete succeeds`
- `two-child delete highlights the immediate successor`
- `SWAP_VALUE carries both ids and both old values`
- `successor is unlinked after the swap`
- `two-child delete ends with DONE`
- `the target id survives and the successor id is gone`
- `the old target key is gone`
- `checker accepts the tree after the immediate-successor delete`
- `deeper two-child delete succeeds`
- `deeper delete highlights the successor`
- `deeper SWAP_VALUE uses both node ids`
- `deeper successor UNLINK carries its right child for undo`
- `successor's right child is relinked to its parent`
- `deeper target id survives with the successor key`
- `checker accepts the tree after the deeper delete`
- `root leaf delete empties the tree`
- `root leaf UNLINK has parent 0 and side NONE`
- `root with one child is replaced by the child`
- `root one-child UNLINK carries the new root id`
- `checker accepts the tree after a root one-child delete`
- `root with two children keeps its id and takes the successor key`
- `root two-child delete removes the successor node`
- `checker accepts the tree after a root two-child delete`
- `missing delete reports the exact message`
- `missing delete leaves the tree unchanged`
- `missing delete ends with DONE`
- `deleting every key drains the tree`
- `empty min and max are null`
- `findMin and findMax return the extremes`
- `findMin follows deletions`
- `findMax follows deletions`
- `successor of a present key`
- `successor works for an absent key`
- `successor below the minimum is the minimum`
- `the maximum has no successor`
- `predecessor of a present key`
- `predecessor works for an absent key`
- `predecessor above the maximum is the maximum`
- `the minimum has no predecessor`
- `empty tree has no successor or predecessor`
- `successor and predecessor agree with the sorted reference`
- `full range returns every key in order`
- `range query returns the inclusive middle band`
- `single-key range works`
- `range outside the tree is empty`
- `range below the tree is empty`
- `inverted range is empty and emits only DONE`
- `range query emits VISIT events`
- `range query highlights pruned subtrees`
- `pruned subtree highlights use the skipped mode`
- `range query ends with DONE`
- `range query on an empty tree emits only DONE`
- `kthSmallest matches the sorted reference for every k`
- `kthSmallest rejects k outside 1..size`
- `kthSmallest on an empty tree is null`
- `lcaBST of siblings is the parent`
- `lcaBST across subtrees is the root`
- `lcaBST of an ancestor and a descendant is the ancestor`
- `lcaBST of the same key is that node`
- `lcaBST with an absent key is null`
- `lcaBST with two absent keys is null`
- `buildChain(0) clears to an empty tree`
- `buildChain(1) creates the root 1`
- `buildChain(5) matches five sequential inserts`
- `buildChain(5) is a five-deep chain`
- `checker accepts buildChain(5)`
- `buildChain(50) is a fifty-deep chain`
- `buildChain(50) is ordered 1..50`
- `checker accepts buildChain(50)`
- `buildChain ids are unique and increase along the chain`
- `buildChain(10000) builds a 10,000-deep chain`
- `10,000-node chain has unique increasing ids and keys`
- `checker accepts buildChain(10000)`
- `10,000-node chain is destroyed`
- `recording off keeps every insert result and an empty log`
- `10,000 sorted inserts build a 10,000-deep chain`
- `10,000 sorted inserts produce increasing ids and keys`
- `10,000-node insert chain is destroyed`
- `500 seeded differential operations match the sorted reference`
- `differential tree size matches the reference`
- `recording off matches recording on and produces empty logs`
- `new ids after a delete are never reused`
- `ids continue after clear`
- `checker accepts the id tree after every mutation`
- `checker accepts the inherited BST after every insert`
- `inherited preorder works on a BST`
- `inherited inorder returns ascending keys`
- `BST deserializes from its own serialization`
- `deserialized BST is valid`

### AVL - 154 checks, 154 passed, 0 failed

- `empty AVL tree measures zero`
- `empty AVL tree is balanced with zero rotations`
- `empty AVL tree answers kth, rank and countInRange`
- `LL insert gives the expected shape`
- `LL insert fixes the root and parent links`
- `LL insert emits the full event sequence`
- `LL insert compares before INSERT`
- `LL insert updates height before size`
- `LL insert alerts on the unbalanced node`
- `LL ROTATE carries pivot, rising node, parent and case`
- `rotation updates the lowered node first`
- `LL insert ends with DONE`
- `LL insert counts one rotation`
- `LL insert passes the checker`
- `RR insert gives the expected shape`
- `RR insert emits one RR rotation`
- `RR pivot is the unbalanced root`
- `RR insert counts one rotation`
- `RR insert passes the checker`
- `LR insert gives the expected shape`
- `LR first rotation pivots the child`
- `LR first rotation updates height before the second rotation`
- `LR second rotation pivots the unbalanced node`
- `LR insert counts two rotations`
- `LR insert passes the checker`
- `RL insert gives the expected shape`
- `RL first rotation pivots the child`
- `RL second rotation pivots the unbalanced node`
- `RL insert counts two rotations`
- `RL insert passes the checker`
- `non-root LL keeps the root`
- `non-root LL relinks the rotated subtree to its parent`
- `non-root LL pivots the inner node`
- `non-root LL passes the checker`
- `non-root RR keeps the root`
- `non-root RR relinks the rotated subtree to its parent`
- `non-root RR pivots the inner node`
- `non-root RR passes the checker`
- `non-root LR keeps the root`
- `non-root LR relinks the rotated subtree to its parent`
- `non-root LR pivots the child first`
- `non-root LR passes the checker`
- `non-root RL keeps the root`
- `non-root RL relinks the rotated subtree to its parent`
- `non-root RL pivots the child first`
- `non-root RL passes the checker`
- `root LL replaces the root and clears its parent`
- `root LL ROTATE records parent 0`
- `root LL passes the checker`
- `root RR replaces the root and clears its parent`
- `root RR pivots the old root`
- `root RR passes the checker`
- `root LR replaces the root and clears its parent`
- `root LR performs two primitive rotations`
- `root LR passes the checker`
- `root RL replaces the root and clears its parent`
- `root RL performs two primitive rotations`
- `root RL passes the checker`
- `ascending inserts stay valid after every insert`
- `1000 ascending inserts give height 10`
- `ascending inserts keep the sorted inorder`
- `descending inserts stay valid after every insert`
- `1000 descending inserts give height 10`
- `1000 random inserts stay balanced within the AVL bound`
- `delete fixture starts balanced`
- `AVL leaf delete removes the key`
- `leaf delete needs no rotation`
- `AVL leaf delete passes the checker`
- `AVL one-child delete bypasses the child`
- `AVL one-child delete passes the checker`
- `AVL two-child delete removes the target key`
- `AVL two-child delete passes the checker`
- `AVL root delete keeps the root id and takes the successor key`
- `AVL root delete passes the checker`
- `AVL missing delete reports the exact message`
- `AVL missing delete leaves the tree and counters untouched`
- `AVL missing delete ends with DONE`
- `AVL duplicate insert reports the exact message`
- `AVL duplicate insert leaves the tree untouched`
- `AVL duplicate insert emits only comparisons and DONE`
- `delete LL fixture loads balanced`
- `delete LL performs a single rotation`
- `delete LL emits one LL rotation at the unbalanced node`
- `delete LL leaves a valid tree rooted at 30`
- `delete RR fixture loads balanced`
- `delete RR performs a single rotation`
- `delete RR emits one RR rotation at the unbalanced node`
- `delete RR leaves a valid tree rooted at 70`
- `delete LR fixture loads balanced`
- `delete LR performs two primitive rotations`
- `delete LR labels both primitives LR in order`
- `delete LR leaves a valid tree rooted at 40`
- `delete RL fixture loads balanced`
- `delete RL performs two primitive rotations`
- `delete RL labels both primitives RL in order`
- `delete RL leaves a valid tree rooted at 60`
- `zero-balance LL fixture loads balanced`
- `left-heavy z with zero-balance left child takes one LL rotation`
- `zero-balance LL result is valid`
- `zero-balance RR fixture loads balanced`
- `right-heavy z with zero-balance right child takes one RR rotation`
- `zero-balance RR result is valid`
- `cascade fixture loads balanced`
- `cascade delete performs at least two rebalances`
- `cascade delete alerts at every unbalanced ancestor`
- `cascade delete leaves a valid tree`
- `500 mixed operations keep kth and rank equal to the reference`
- `countInRange matches the reference for all bound pairs`
- `countInRange covers the full range without overflow`
- `countInRange returns zero for inverted and empty ranges`
- `countInRange on an empty tree is zero`
- `manual rotateLeft moves the right child up and fixes the root`
- `manual rotateLeft keeps every parent link`
- `manual rotateLeft emits a standalone RR rotation`
- `manual rotateLeft emits ROTATE before UPDATE events and ends with DONE`
- `a manual rotation may leave the tree unbalanced`
- `manual rotateLeft keeps stored stats correct`
- `manual rotateLeft creates and destroys no nodes`
- `manual rotateRight restores the earlier shape`
- `manual rotateRight keeps stored stats correct`
- `manual rotateLeft rejects a missing id`
- `manual rotateLeft rejects a node without a right child`
- `manual rotateRight rejects a node without a left child`
- `deserialize recomputes height and size on load`
- `buildFromPreorderInorder recomputes stats`
- `buildFromPostorderInorder recomputes stats`
- `checker rejects corrupted stored stats`
- `recomputeStats repairs corrupted height and size`
- `isBalanced accepts a balanced tree`
- `isBalanced rejects a right chain`
- `isBalanced ignores a corrupted stored height`
- `checker still sees the corrupted height`
- `isBalanced ignores a corrupted stored size`
- `recomputeStats fixes the size again`
- `ids after delete and re-insert are never reused`
- `ids continue after clear`
- `rotations preserve the full id set`
- `recording off matches recording on with empty logs`
- `counters start at one rotation after LL`
- `lastRotations resets when an insert needs no rotation`
- `rotationCount is cumulative`
- `a double rotation adds two to both counters`
- `an insert without rotation leaves the cumulative count alone`
- `manual rotations count as one primitive rotation`
- `cumulative count survives later operations`
- `10,000 sorted AVL inserts give height 14`
- `10,000-node AVL tree passes the checker`
- `10,000 sorted deletes drain the AVL tree`
- `BST API fixture loads`
- `search works on a rotated AVL tree through the BST API`
- `rangeQuery works on a rotated AVL tree through the BST API`
- `successor and predecessor work through the BST API`
- `kthSmallest works through the BST API`
- `lcaBST works on a rotated AVL tree`

### Tree validator - 24 checks, 24 passed, 0 failed

- `validator accepts empty trees`
- `validator accepts a null raw root`
- `validator accepts a clean BST`
- `validator accepts a clean AVL tree`
- `validator is read-only on a BST`
- `validator is read-only on an AVL tree`
- `validator catches swapped keys as an ordering problem`
- `restoring swapped keys clears the problem`
- `validator catches a duplicated key as an ordering problem`
- `validator catches a wrong stored height`
- `restoring the height clears the problem`
- `validator catches a wrong stored size`
- `restoring the size clears the problem`
- `validator catches a broken parent link`
- `restoring the parent link clears the problem`
- `validator catches a child cycle without looping`
- `breaking the cycle clears the problem`
- `validator catches a node shared by two parents`
- `restoring the shared child clears the problem`
- `validator catches an AVL balance violation`
- `validator does not impose BST ordering on a general tree`
- `raw root overload honours the ordering flag`
- `clean BST passes the validator after every differential operation`
- `clean AVL tree passes the validator after every differential operation`

### Tree layout - 20 checks, 20 passed, 0 failed

- `layout of an empty tree is empty with a zero box`
- `empty layout has no positions`
- `layout of a single node has one position`
- `single node sits at the origin`
- `single node has a zero bounding box`
- `skewed tree steps one column and one level per node`
- `skewed tree positions do not overlap`
- `skewed tree bounding box spans every node`
- `full tree layout has one position per node`
- `full tree positions do not overlap`
- `full tree root sits at the horizontal centre`
- `full tree bounding box covers four levels`
- `200-node random layout keys every node id`
- `200-node random positions do not overlap`
- `mirrored tree gives mirrored x positions`
- `AVL tree layout has one non-overlapping position per node`
- `raw root overload matches the tree overload`
- `10,000-node skewed layout is iterative and complete`
- `10,000-node skewed bounding box spans the full height`
- `non-positive spacing is clamped to one`

## Heap results (Arslan)

73 checks, 73 passed, 0 failed. Captured from `tests.exe` on 21 November 2026.

### MaxHeap model

- `empty heap builds safely`
- `single value is already a heap`
- `buildMaxHeap repairs an ordinary array`
- `HEAP_BUILD records last parent and heap size`
- `build swaps record the BUILD reason`
- `build counts comparisons and swaps`
- `building an existing heap does not swap values`
- `duplicates are allowed in a Max Heap`
- `buildMaxHeap repairs a reverse-sorted array`
- `buildMaxHeap repairs a random permutation`
- `insert uses heapify up`
- `heap insertion reuses INSERT with index and value`
- `insert records the old and new heap sizes`
- `extractMax moves the maximum into the sorted region`
- `extraction records HEAP_SIZE_CHANGE using oldValue and newValue`
- `extraction swap records its indices and reason`
- `insert is rejected while a sorted region exists`
- `repeated extraction returns values from largest to smallest`
- `Heap Sort produces ascending order and marks the whole array sorted`
- `Heap Sort ends with completion and done events`
- `Heap Sort matches reference sort on an empty array`
- `Heap Sort matches reference sort on a singleton`
- `Heap Sort matches reference sort with duplicates`
- `Heap Sort matches reference sort with negatives`
- `Heap Sort matches reference sort on a sorted array`
- `Heap Sort matches reference sort on a reverse-sorted array`
- `Heap Sort matches reference sort on a random array`
- `recording off keeps heap logic working without events`
- `heap snapshot restores values and active boundary for undo`
- `invalid heap snapshot is rejected without changing state`
- `non-integer heap snapshot token is rejected`
- `negative heap size snapshot is rejected`
- `empty values with positive heap size are rejected`
- `empty heap snapshot restores cleanly`
- `extractMaxNoHeapify moves the max without restoring the heap`
- `repairDown restores the heap after extractMaxNoHeapify`

### Heap validator

- `validator accepts a valid active heap and sorted region`
- `validator catches a larger left child`
- `validator catches a larger right child`
- `validator catches a heap size outside the array`
- `validator catches mismatched heap and sorted boundaries`
- `validator catches an unsorted tail`
- `validator catches a sorted value smaller than the active maximum`
- `validator never changes the array`
- `validator accepts a built MaxHeap object`
- `validator accepts a partially extracted heap`
- `validator accepts a completed Heap Sort`

### Heap view

- `heap view loads one shared visual state`
- `heap view animates a swap before changing values`
- `heap view completes a swap once`
- `heap view flags both ends of a broken parent child pair`
- `heap view reset restores the starting snapshot`
- `heap view follows insertion events`

### Act 4 levels

- `heap build level starts with a broken raw heap`
- `invalid heap swap adds one alarm penalty`
- `valid heap swap accepts parent child selection in either order`
- `valid player swap changes the model and records its reason`
- `required swaps complete the heap build level`
- `heap build snapshot does not change the model`
- `repair level recognizes both valid directions on a broken edge`
- `wrong repair direction adds one alarm penalty`
- `heapify down repairs the disturbed root and completes the level`
- `valid heapify up may improve the heap without completing it`
- `a longer valid repair route is allowed but costs an extra step`
- `extract leaves the heap needing a restore`
- `extract counts as one player step`
- `heapify restores the Max Heap after extract`
- `undo restores heap, score, and awaiting-heapify state`
- `second undo restores the pre-extract snapshot`
- `extract level completes after every maximum is removed and restored`
- `race level completes after the configured extraction count`
- `Heap Sort finale completes by manual extract and heapify steps`
- `heapify before extract is an invalid heap action`

_This section is intentionally empty._

