# Tree validator (task A4)

`vh::TreeValidator` is read-only. It never rewrites the tree, so a caller can run
it on the live model at any time. It lives next to `HeapValidator` in
`src/model/tree/`; Arslan's B2 `validate()` front door currently only exposes the
heap overloads, so this task adds the standalone `validate` / `validateTree`
entry points instead of editing the heap files. A combined front door can call
`TreeValidator::validate` once B2 grows a shared `validate(structure)` overload.

## API

| Call | Result |
| --- | --- |
| `validate(const BinaryTree&)` | Problems for the whole tree. Ordering is checked for a `BST`/`AVLTree`, AVL fields for an `AVLTree`. |
| `validateTree(const BinaryTree&)` | Same checks, explicit tree overload. |
| `validateTree(const TreeNode* root, bool checkOrdering, bool checkAVL)` | Checks a raw root; the caller chooses the rules. A `nullptr` root is valid. |

A result is a `TreeValidationResult` with `ok()` and a `DynArray<TreeProblem>`.
Each problem carries a code, the node id, and a readable message:

| Code | Meaning |
| --- | --- |
| `TreeProblemCode::ORDER` | inorder keys are not strictly ascending (swapped or duplicated key) |
| `TreeProblemCode::VALUE` | a BST node's `key` and `value` differ |
| `TreeProblemCode::HEIGHT` | stored `height` differs from the recomputed height |
| `TreeProblemCode::SIZE` | stored `size` differs from the recomputed size |
| `TreeProblemCode::BALANCE` | `|balance factor| > 1` |
| `TreeProblemCode::PARENT` | a child's `parent` link is not the node that reached it |
| `TreeProblemCode::CYCLE` | a node is its own ancestor |
| `TreeProblemCode::SHARED` | the same node id is reached twice from different parents |

## How cycles and shared nodes are detected

The validator does one iterative depth-first walk. It keeps a hash set of node
ids that have already been expanded and a stack of the nodes on the current
path. When an id is seen again, the node is on the path exactly when the walk
has looped back to an ancestor, so the problem is reported as `CYCLE`;
otherwise the same node was reached from a second parent and is reported as
`SHARED`. Either way the repeated node is not expanded again, so every node is
visited at most once and the walk always terminates, even for a malformed tree.
The set is an open-addressing table built on `DynArray<int>`; no STL container
or algorithm is used. Because the walk is bounded by the number of distinct ids,
the traversal count is the number of expanded nodes. A clean tree expands every
node exactly once; a duplicate means the walk found a cycle or a shared node.

Ordering and AVL checks run only when the walk found no cycle or shared node,
because both use child pointers. A wrong parent link does not stop those checks.

## Checks

- **Ordering:** an iterative inorder walk verifies strict ascending keys and
  `key == value`. A duplicated key is therefore an ordering problem.
- **Height and size (AVL only):** one iterative postorder recomputes height and
  size from the real shape and compares them with the stored fields. Stored
  values are never trusted.
- **Balance (AVL only):** the same postorder compares the recomputed child
  heights at every node.
- **Parent links:** during the walk every child must point back to the node that
  reached it. The root must have a `nullptr` parent.

The validator is O(n) time and O(n) extra space, and it is iterative, so a
10,000-node tree does not risk the call stack.
