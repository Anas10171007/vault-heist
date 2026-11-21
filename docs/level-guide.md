# Level guide

## Where levels live

One JSON file per act: `data/levels/act1.json`, `act2.json`, `act3.json`,
`act4.json`. Each file is `{"version": 1, "act": N, "levels": [ ... ]}`.
`LevelManager::loadAll("data/levels")` reads all four files; `load("2.3")` returns
one definition by id. Levels are plain data: adding one means adding an object
to the `levels` array, with no code change.

JSON here is deliberately small: objects, arrays, strings, whole numbers and
`true`/`false`. No comments, decimals, `null`, backslashes or double quotes
inside strings (use single quotes in text). Files are UTF-8 without a BOM.
Unknown fields are ignored.

## Level fields

| Field | Meaning |
| --- | --- |
| `id` | Unique string across all files, starting with the act number, e.g. `"2.6"`. |
| `title` | Short level title. |
| `mission` | Text shown in the briefing. |
| `structure` | One of `binary`, `bst`, `avl`, `maxheap`. |
| `task` | Must belong to the structure (table below). |
| `seed` | Whole number; same seed rebuilds the same generated data and picks optional rules. |
| `data` | Data source object, described below. |
| `rules` | Object of optional task rules; `{}` is allowed. |

`LevelManager` rejects a wrong file version or act number, a missing required
field, a duplicated id, an id whose prefix does not match the act, a task that
does not belong to the structure, and missing or contradictory data fields.
`LevelManager::error()` then names the level and the problem.

## `data`

`source` is `explicit` or `generate`.

- `explicit`: exactly one of
  - `tree`: a preorder string with `#` for an empty child, e.g.
    `"50 30 # # 70 60 # # #"`. Loaded exactly as written, no rebalancing.
    Required for a `binary` structure.
  - `values`: an array of whole numbers. `bst` inserts them one by one without
    rebalancing, `avl` inserts them one by one with rebalancing, `maxheap` uses
    them as the raw array. Not allowed for `binary`.
- `generate`:
  - `shape`: `random`, `sorted`, `reverse` for tree structures; `duplicates`,
    `heap`, `disturbed` are heap-only.
  - `size`: number of nodes or elements (positive).
  - `min`, `max`: inclusive value range (defaults 1 to 99).
  - `disturb`: parent-child swaps for the `disturbed` heap shape.

## Task names by structure

| Structure | Valid `task` values |
| --- | --- |
| `binary` | `traversal_click`, `reconstruct`, `height_diameter`, `lca_path`, `mirror` |
| `bst` | `bst_search`, `bst_insert`, `bst_delete`, `bst_range`, `bst_boss` |
| `avl` | `avl_badges`, `avl_rotate_single`, `avl_rotate_double`, `avl_auto_predict`, `avl_kth` |
| `maxheap` | `heap_build`, `heap_repair`, `heap_extract`, `heap_race`, `heap_sort` |

### Rules by task

Every rule field is optional; a missing one is chosen from the seed by the
level logic.

| Task | Rule fields |
| --- | --- |
| `traversal_click` | `traversal`: `preorder`, `inorder`, `postorder`, `level` |
| `reconstruct` | `from`: `pre_in` or `post_in` (values must be distinct) |
| `height_diameter` | none |
| `lca_path` | `a`, `b` (node values) |
| `mirror` | none |
| `bst_search`, `bst_insert`, `bst_delete` | `target` (key) |
| `bst_range` | `lo`, `hi` |
| `bst_boss` | none; use `generate` with `shape: "sorted"` |
| `avl_badges` | none |
| `avl_rotate_single` | `rotation`: `LL` or `RR` |
| `avl_rotate_double` | `rotation`: `LR` or `RL` |
| `avl_auto_predict` | `inserts`: array of keys inserted one by one |
| `avl_kth` | `k` |
| `heap_build`, `heap_repair`, `heap_sort` | none |
| `heap_extract`, `heap_race` | `count`: number of extractions |

## Copy-and-edit example

To add a sixth Act 2 level, append this object to the `levels` array in
`data/levels/act2.json` (remember the comma after the previous object):

```json
{
  "id": "2.6",
  "title": "Find the courier",
  "mission": "Search for the courier key before the alarm fills.",
  "structure": "bst",
  "task": "bst_search",
  "seed": 2606,
  "data": {"source": "explicit", "values": [50, 30, 70, 20, 40, 60, 80]},
  "rules": {"target": 40}
}
```

Keep the id prefix (`2.`) in the same act file, keep the id globally unique,
and make sure `target` is a key that exists for `bst_search` / `bst_delete` or
does not exist for `bst_insert`. Change the seed only when you want different
generated data.

## What is not stored

Optimal steps, alarm capacity, star limits and win checks are computed by the
model and `ScoreSystem`, never typed into the JSON. The level logic derives
optimal steps from the tree it builds: traversal length, search or insert path
length, the number of repair rotations, the keys in a range, or the number of
kth stages. Capacity is `ceil(1.5 x optimal) + 2`. See
`docs/level-interface.md` for what the screens call.
