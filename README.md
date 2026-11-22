# Vault Heist: The Living Vault

Data Structures project by Anas and Arslan. C++17, CMake, SFML 3.0.0, MinGW-W64
GCC 14 on Windows. SFML is fetched by CMake at configure time.

## Build

Run from the `vault-heist` folder:

```powershell
cmake -S . -B build
cmake --build build -j 4
```

The first configure downloads SFML 3.0.0. If GitHub cloning fails or is slow,
clone SFML yourself and point FetchContent at the local checkout:

```powershell
cmake -S . -B build -DFETCHCONTENT_SOURCE_DIR_SFML=C:/path/to/SFML
```

Use a checkout of the 3.0.0 tag. If the source directory is already populated
and you want to stay offline, add `-DFETCHCONTENT_FULLY_DISCONNECTED=ON`.
Reconfigure with `cmake -S . -B build` after pulling changes; CMake globs source
files with `CONFIGURE_DEPENDS`.

Building `vaultheist` copies `data/` to `build/bin/data`. The font loader also
checks repository-relative paths, so run executables from the repository root.

In VS Code use **Run and Debug > Run Vault Heist**. The single-file
**Compile & Run** button cannot build this multi-file CMake project.

## Run

All paths below are relative to the repository root:

| Command | What it does |
| --- | --- |
| `.\build\bin\vaultheist.exe` | Game shell (menu, campaign map, briefing, Act 4 level play). |
| `.\build\bin\tests.exe` | Full unit test suite; prints one `PASS`/`FAIL` line per case. |
| `.\build\bin\bench.exe` | Generates seven deterministic CSVs in `bench/results`. |
| `.\build\bin\renderdemo.exe` | Renderer demo. |
| `.\build\bin\heapviewdemo.exe` | Heap tree + array view demo. |
| `.\build\bin\treeanimdemo.exe` | Tree animation demo; `--smoke` opens, plays one insert, saves a PNG and closes. |
| `.\build\bin\levelsmoke.exe` | Logic smoke run of every level (`--logic` for a windowless check). |

## How to play

### Tree levels (Acts 1–3)

- **Clicking the tree:** rooms/nodes are played by clicking them.
  Traversal levels expect clicks in the traversal order (pre, in, post or
  level); a win is when the click sequence equals the traversal.
  Search levels (2.x) navigate left or right at each room to find the target;
  the alarm counts comparisons. Act 2's 2.4 is a range query `[lo, hi]` —
  skipped subtrees grey out and the score rewards pruning.
- **Token placement (1.2, 1.4–1.5):** drag the room tokens into tree slots
  using pre + in (and post + in); the built tree must equal the hidden tree.
  Select two rooms for an LCA check and trace the path (1.4). 1.5 mirrors the
  vault by swapping children bottom-up (or **Auto**).
- **Enter:** type digits, then Enter submits the numeric answer.
- **Hint:** `H` — model-computed hint; using a hint caps the result at two
  stars.
- **Undo:** `Z`. **Reset:** `R`. **Space / Left / Right** play/pause and step
  the event animation.
- **Esc:** back to the Campaign Map.
- **3.x:** Act 3's AVL levels show balance badges (green for −1, 0, 1, red for
  a violation); click all unbalanced rooms. Manual repair uses named pivots:
  LL and RR by dragging a pivot and selecting Rotate Left/Right, LR and RL by
  naming the case then performing both rotations. 3.4 is the auto-repair vault
  that repairs itself with animation. 3.5 steals the k-th most valuable
  treasure using subtree sizes.
- **2.3:** the player first picks the case (leaf, one child, two children),
  then the two-child case highlights the inorder successor and shows the value
  swap.

### Endless

A seeded stream of mixed inserts and deletes on an AVL; the tree must stay
valid. The run ends when the validator rejects the tree, or after 25
operations (`MAX_OPS`). Ops apply on a ramping timer: the interval starts at
1.6 s and shortens by 30 ms per operation, floored at 0.5 s. **Enter** inserts
a typed value (20–90). Scores are kept in memory only — they are not saved to
any persistent leaderboard.

### Sandbox

A free-form toolbox over the tree structures. `Structure` cycles through
**Binary Tree → BST → AVL → Max Heap**.

- **Binary Tree / BST / AVL:** Insert, Delete, Search, and (for BST/AVL)
  Range; AVL adds Rotate L/R; Random and Clear are available in all tree modes.
- **Max Heap:** Insert, Extract (extracts the max), and **Build Max Heap**.
  There is no separate Heapify/Heap Sort button — that behavior is covered in
  Act 4.

### Known limitations

- No level locks: the Campaign shows every mission open (Acts 1–4).
- Endless runs are not saved anywhere — scores live in memory only.
- Manual AVL rotations are instant (no detailed animation); the replay
  scrubber exists only on Act 4's heap screen.

## Controls

### Game (`vaultheist`)

- **Mouse:** left-click navigation buttons on every screen.
- **Esc:** return to the Main Menu. Closing the window quits.
- **Act 4 Level Play:** left-click a node in the heap tree or a cell in the array
  to pick two indices for a swap. Buttons: **Reset Level**, **Menu**, and
  **Continue** when the level is complete.
- **Act 4 keyboard:** `R` reset, `Space` play/pause animations, `Left`/`Right`
  step animations, `Enter` continue when complete.

The Campaign Map, Briefing, Result, Sandbox, Endless, Leaderboard and Settings
screens are still stubs (buttons only). Act 1-3 tree level screens, the result
stats view, sandbox, endless mode, leaderboard persistence and settings are
**planned**. Their model logic already exists in `src/game/act1`..`act3` and is
covered by unit tests, but no GUI screen calls it yet. The animation demo screen
has `Up`/`Down` speed and `I` instant-finish handlers in code, but that screen is
not registered in `ScreenManager` yet, so those keys are **planned** in game.

### Tree animation demo (`treeanimdemo`)

`1`-`4` play real AVL inserts that trigger LL, RR, LR and RL. `I` inserts,
`D` deletes, `Space` plays/pauses, `Left`/`Right` step, `P` saves
`screenshots/tree_anim.png`, `Esc` quits. `--smoke` runs a single scenario and
closes automatically.

## Folder map

| Path | Contents |
| --- | --- |
| `src/model` | GUI-free nodes, events, `EventLog`, `tree/` (BinaryTree, BST, AVLTree, layout, motion, validator) and `heap/` (MaxHeap, HeapValidator). |
| `src/util` | `DynArray`, `Stack`, `Queue`, `Rng`, `NodeId`. |
| `src/game` | `LevelDef`, `LevelManager`, `JsonReader`, `ScoreSystem`, `LevelPlay` + factory, `LevelDataBuilder`, `tree/`, and `act1/`..`act4/` pure logic. |
| `src/gui/core` | `Renderer`, `Style`/`Palette`, `AnimationController`, `Theme`, `Camera`, `ScreenManager`. |
| `src/gui/screens`, `src/gui/widgets` | Screens and the heap view widget. |
| `src/gui/tree` | `TreeAnimationView` (the A6 event handlers). |
| `tests` | `test_main.cpp` plus `tree/`, `heap/` and `game/` suites. |
| `data/levels` | `act1.json`..`act4.json`; see `docs/level-guide.md`. |
| `tools` | `tree_anim_demo.cpp`, `heap_view_demo.cpp`, `render_demo.cpp`. |
| `docs` | Feature and task notes. `tests/TEST_REPORT.md` is generated from real test output. |
| `assets/fonts` | `Inter-Regular.ttf` and its licence. |
| `screenshots` | PNG written by `treeanimdemo` `P` / `--smoke`. |
| `bench` | Deterministic BST, AVL, heap, Heap Sort, and quadratic-sort benchmarks. |

## Tests

Run from the repository root:

```powershell
.\build\bin\tests.exe
```

The suite prints `PASS <case>` / `FAIL <case>` and ends with `all passed` when
everything succeeds. `tests/TEST_REPORT.md` records the exact totals from a
real run. `bench.exe` generates seven deterministic CSV files in
`bench/results`; see `docs/bench-notes.md` for methodology.

## Documentation

`docs/LLM-HANDOFF.md` records the team workflow, coding constraints, and
current handoff status for any new AI assistant. `docs/level-format.md` is the
JSON reference, `docs/level-guide.md` is the practical guide to adding levels,
`docs/level-interface.md` documents the API the level screens will call, and
`docs/event-payloads.md` documents event fields.
