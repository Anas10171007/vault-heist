# Tree layout engine (task A5)

`vh::TreeLayout` turns a tree shape into plain floats. It lives in
`src/game/tree/` so it is part of `gamecore` and tests can link it without SFML.
It never draws, never includes a GUI header, and keeps no state: `compute` is a
static call that returns a fresh value, so the current tree can be laid out
before a rotation animation starts and the same call can produce the
post-rotation layout afterwards.

## API

| Call | Result |
| --- | --- |
| `TreeLayout::compute(const BinaryTree& tree, float xSpacing = 64.f, float ySpacing = 90.f)` | Layout for a whole tree. Works for `BinaryTree`, `BST` and `AVLTree`. |
| `TreeLayout::compute(const TreeNode* root, float xSpacing, float ySpacing)` | Same layout from a raw root. `nullptr` gives an empty layout. |
| `size()`, `empty()` | Number of positioned nodes. |
| `contains(id)` | Whether the id has a position. |
| `at(index)` | `LayoutPoint { nodeId, x, y }` in inorder order. |
| `xOf(id)`, `yOf(id)` | Keyed lookup; a missing id returns 0. |
| `bounds()` | `LayoutBox { minX, minY, maxX, maxY }` plus `width()` and `height()`, ready for `fitToView`. |

Positions are keyed by node id, not by key, so a layout stays valid when keys
are swapped.

## Algorithm

One iterative depth-first walk assigns both coordinates:

- `x` is the node's column in the inorder sequence times `xSpacing`. The first
  node visited is the leftmost, so `x` starts at 0.
- `y` is the node's depth times `ySpacing`, with the root at 0.

Every node gets a different column, so two nodes can never share an `x`, and
any two nodes are at least `xSpacing` apart horizontally. Siblings, and nodes
from different subtrees, therefore never overlap. Because the walk is inorder,
edges do not cross.

A mirrored tree has the reversed inorder sequence, so its layout is the mirror
of the original around the box centre:
`xMirrored(id) = width - xOriginal(id)`, while `y` is unchanged.

Non-positive spacing values are clamped to 1.

## Complexity

`n` is the node count.

| Operation | Time | Space |
| --- | --- | --- |
| `compute` | O(n) | O(n) positions plus an explicit stack |
| `contains`, `xOf`, `yOf` | O(n) | O(1) |

The walk uses an explicit stack, so a 10,000-node skewed tree does not recurse
and cannot overflow the call stack. The bounding box is accumulated during the
same walk, so no second pass is needed.
