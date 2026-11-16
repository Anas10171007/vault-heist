# Level file format (v1)

One JSON file per act: `data/levels/act1.json` to `act4.json`. Each file is `{"version": 1, "act": N, "levels": [ ... ]}`. Adding a level means adding one object to the list. No code change.

## Parser limits
Objects, arrays, strings, whole numbers (negative allowed) and true/false. No null, no decimals, no comments. Strings contain no backslash and no double quote (use single quotes inside text). Files are UTF-8 without a BOM. Unknown fields are ignored. A missing required field gives a message naming the level id and the field.

## Level fields (all required)
- `id` string, unique across all files, starts with the act number, for example "2.3"
- `title` string
- `mission` string shown in the briefing
- `structure` one of `binary`, `bst`, `avl`, `maxheap`
- `task` one of the task names in the table below, and it must belong to the structure
- `seed` whole number. Same seed gives the same level on any machine. Also used to pick any optional rule that is left out.
- `data` object, see below
- `rules` object, may be empty `{}`

## data
`source` is `explicit` or `generate`.

explicit: exactly one of
- `tree`: preorder with `#` for an empty child, for example `"50 30 # # 70 60 # # #"`. Loaded exactly as written, no rebalancing. Heights and sizes are recomputed.
- `values`: whole numbers. `bst` inserts them one by one (plain). `avl` inserts them one by one with rebalancing. `maxheap` uses them as the raw array, not yet heapified. Not allowed for `binary`.

generate:
- `shape`: `random`, `sorted`, `reverse`, `duplicates` (maxheap only), `heap` (already a valid heap), `disturbed` (a valid heap with `disturb` pairs swapped, maxheap only)
- `size`: number of nodes or elements
- `min`, `max`: value range (default 1 to 99)
- `disturb`: how many parent-child pairs to swap, used with `disturbed`

## rules by task
Every rule field is optional. A missing one is chosen from the seed.

binary
- `traversal_click`: `traversal` = preorder, inorder, postorder or level
- `reconstruct`: `from` = pre_in or post_in (values must be distinct)
- `height_diameter`: none
- `lca_path`: `a`, `b` (node values)
- `mirror`: none

bst
- `bst_search`: `target`
- `bst_insert`: `target` (a key not yet in the tree)
- `bst_delete`: `target`
- `bst_range`: `lo`, `hi`
- `bst_boss`: none. Use shape `sorted`

avl
- `avl_badges`: none
- `avl_rotate_single`: `rotation` = LL or RR
- `avl_rotate_double`: `rotation` = LR or RL
- `avl_auto_predict`: `inserts` = list of keys to insert one by one
- `avl_kth`: `k`

maxheap
- `heap_build`, `heap_repair`, `heap_sort`: none
- `heap_extract`, `heap_race`: `count` = number of extractions

## Not stored
Optimal steps, alarm capacity, star limits and win checks are computed by the model and the ScoreSystem. Endless mode has no level file.