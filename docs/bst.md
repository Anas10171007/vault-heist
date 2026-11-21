# BST model (task A2)

`vh::BST` derives from `vh::BinaryTree`. Every node is a `BSTNode`; `key` is
authoritative and `value` always equals `key`, including after the value swap in
a two-child delete. Duplicate keys are rejected. All operations are iterative
and keep parent pointers correct.

## API

| Operation | Result | Description |
| --- | --- | --- |
| `insert(key)` | `OpResult` | Standard insert. Duplicates fail with `duplicate key K rejected` and leave the tree unchanged. |
| `search(key)` | `SearchResult` | Returns the node or `nullptr`; emits one COMPARE per visited node. |
| `deleteKey(key)` | `OpResult` | Leaf, one-child and two-child cases. A missing key fails with `key K not found` and leaves the tree unchanged. |
| `findMin()` / `findMax()` | `BSTNode*` | Leftmost / rightmost node, `nullptr` on an empty tree. |
| `successor(key)` / `predecessor(key)` | `BSTNode*` | Smallest key strictly greater / largest key strictly smaller. Top-down, works for absent keys. |
| `rangeQuery(lo, hi)` | `RangeResult` | Ascending keys in `[lo, hi]`; `lo > hi` gives an empty result. |
| `kthSmallest(k)` | `BSTNode*` | 1-based inorder counter, `nullptr` when `k` is outside `1..size`. |
| `lcaBST(keyA, keyB)` | `BSTNode*` | Lowest common ancestor by ordering; `nullptr` if either key is absent. |
| `buildChain(n)` | `void` | Builds the right-leaning chain `1..n` in O(n), without comparisons or events. `n <= 0` clears the tree. |

`insert` and `deleteKey` are virtual so `AVLTree` can extend them. `createNode`
is overridden to produce `BSTNode` objects, so `deserialize` and reconstruction
also create BST nodes.

`detachNode` requires a node with at most one child. The protected hooks
`afterInsert(inserted, events)` and `afterRemoval(parentOfRemoved, events)` are
no-ops in BST and exist for AVLTree rebalancing.

## Events per operation

- `insert`: COMPARE for each node on the descent, then INSERT(new id, parent id,
  side, key), then DONE. An empty tree emits INSERT with parent 0 and side NONE.
  A duplicate emits COMPARE events ending with `extra = 0`, no INSERT, then DONE.
- `search`: COMPARE for each visited node, then DONE.
- `deleteKey`: COMPARE along the path (missing key: those COMPAREs then DONE),
  then:
  - leaf: UNLINK(node, parent, side, key), DONE.
  - one child: UNLINK(node, parent, side, key) with the bypassed child id in
    `extra`, DONE.
  - two children: HIGHLIGHT_SUBTREE(successor, result), SWAP_VALUE(target,
    successor, old target value, old successor value), then the successor's
    leaf/one-child UNLINK, DONE.
- `rangeQuery`: VISIT for each visited node, HIGHLIGHT_SUBTREE(child, skipped)
  when a subtree is pruned, DONE.

With `setRecordEvents(false)` every operation returns the same values and an
empty log.

## Complexity

`h` is the height in nodes (empty tree 0, leaf 1); `k` is the number of keys
returned by a range query.

| Operation | Time |
| --- | --- |
| `insert`, `search`, `deleteKey` | O(h) |
| `findMin`, `findMax` | O(h) |
| `successor`, `predecessor` | O(h) |
| `rangeQuery` | O(h + k) |
| `kthSmallest` | O(n) |
| `lcaBST` | O(h) |
| `buildChain` | O(n) |
