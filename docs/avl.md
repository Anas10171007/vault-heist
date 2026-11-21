# AVL tree model (task A3)

`vh::AVLTree` derives from `vh::BST`. Every node is an `AVLNode`, so it also
stores `height` and `size`. Height counts nodes (empty 0, leaf 1), a leaf has
size 1, and every node keeps `|balanceFactor| <= 1`, where
`balanceFactor(node) = height(left) - height(right)`.

`insert` and `deleteKey` are inherited from `BST`. `AVLTree` overrides only the
protected hooks `afterInsert` and `afterRemoval` plus `createNode`; every
insert and delete therefore rebalances by walking from the changed parent to
the root through parent pointers. No operation in `AVLTree` recurses.

## API

| Operation | Result | Description |
| --- | --- | --- |
| `AVLTree(recordEvents = true)` | | Empty tree. Counters start at 0. |
| `insert(key)` | `OpResult` | Inherited BST insert, then rebalance. Duplicates are rejected unchanged. |
| `deleteKey(key)` | `OpResult` | Inherited BST delete, then rebalance from the parent of the physically removed node. |
| `rotateLeft(nodeId)` / `rotateRight(nodeId)` | `OpResult` | Manual primitive for Act 3. Fails with `node I not found`, `node I has no right child` or `node I has no left child`. Does not rebalance further. |
| `balanceFactor(node)` | `int` | Stored-height difference; 0 for `nullptr`. |
| `kth(k)` | `BSTNode*` | 1-based k-th smallest using subtree sizes, O(log n). `nullptr` when `k < 1` or `k > size`. |
| `rank(key)` | `int` | Number of keys `<= key`, O(log n). Works for absent and out-of-range keys. |
| `countInRange(lo, hi)` | `int` | `rank(hi)` minus the number of keys `< lo`, O(log n). No overflow at `INT_MIN`; `lo > hi` gives 0. |
| `isBalanced()` | `bool` | Iterative postorder over the real shape. It recomputes heights and never trusts stored `height` or `size`. |
| `recomputeStats()` | `void` | Iterative postorder that rewrites `height` and `size` bottom-up, with no events. |
| `rotationCount()` | `int` | Cumulative primitive rotations since construction. A double rotation adds 2. |
| `lastRotations()` | `int` | Primitive rotations performed by the most recent successful insert or delete. |
| `deserialize(text)` | `BuildResult` | Hides the `BinaryTree` version and calls `recomputeStats()` on success. |
| `buildFromPreorderInorder(pre, in)` | `BuildResult` | Same hiding and recompute. |
| `buildFromPostorderInorder(post, in)` | `BuildResult` | Same hiding and recompute. |

`clear()` does not reset either rotation counter. Calling a hiding load
function through a `BinaryTree&` bypasses `recomputeStats`, because the base
functions are not virtual.

## Events per operation

Both hooks call `updateStats` on every node from the start node to the root.
`updateStats` emits `UPDATE_HEIGHT(node, old, new)` only when the height
changes, then `UPDATE_SIZE(node, old, new)` only when the size changes.

- `insert`: the BST `COMPARE` events, `INSERT`, then the upward walk. At a node
  with `|bf| > 1` it emits `BALANCE_ALERT(node, bf)`, performs the rotation(s),
  and continues from the new subtree root's parent. `DONE` is last.
- `deleteKey`: the BST `COMPARE` events and the delete events, then the same
  upward walk starting at the parent of the physically removed node (the target,
  or the inorder successor in a two-child delete). The parent may be 0 for a
  removed root. `DONE` is last.
- A duplicate insert or missing delete emits no rebalance events.

Case selection for an unbalanced node `z`:

| Condition | Case | Rotation |
| --- | --- | --- |
| `bf(z) > 1`, `bf(z->left) >= 0` | LL | right at `z` |
| `bf(z) > 1`, `bf(z->left) < 0` | LR | left at `z->left`, then right at `z` |
| `bf(z) < -1`, `bf(z->right) <= 0` | RR | left at `z` |
| `bf(z) < -1`, `bf(z->right) > 0` | RL | right at `z->right`, then left at `z` |

The `>= 0` and `<= 0` comparisons are the deletion rule: a heavy child with
balance factor 0 takes a single rotation.

## ROTATE payload

| Field | Meaning |
| --- | --- |
| `idA` | pivot: the node that moves down |
| `idB` | node that becomes the subtree root |
| `key` | parent id of the subtree before the rotation (0 for the root) |
| `extra` | case: `RotationType::LL`, `RR`, `LR` or `RL` |

Each primitive rotation emits one `ROTATE` before its own `UPDATE_*` events.
Both primitives of a double rotation carry the same case name, so LR emits two
`LR` rotations in order. A standalone `rotateRight` is labelled `LL` and a
standalone `rotateLeft` is labelled `RR`. After relinking, stats are updated
for the node that moved down first, then for the node that moved up.

## Complexity

`n` is the node count, `h` the height.

| Operation | Time |
| --- | --- |
| `insert`, `deleteKey` | O(h) = O(log n), including the walk and rotations |
| `kth`, `rank`, `countInRange` | O(log n) |
| `rotateLeft`, `rotateRight` | O(1) |
| `isBalanced`, `recomputeStats` | O(n) |

Recording off returns identical results, shapes and counters with empty logs.
