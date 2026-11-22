# Vault Heist: The Living Vault

Data Structures project by Anas (25I-3062) and Arslan (25I-3101).
C++17, CMake, SFML 3.0.0, MinGW-W64 GCC 14 on Windows. SFML is fetched by CMake
at configure time.

Written report: `docs/Project_Report.md` (and `docs/Project_Report.html` for
Print → PDF). Screenshots live in `screenshots/`.

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

Building `vaultheist` copies `data/` and `assets/` to `build/bin/`. The font
loader also checks repository-relative paths, so run executables from the
repository root.

In VS Code use **Run and Debug > Run Vault Heist**. The single-file
**Compile & Run** button cannot build this multi-file CMake project.

## Run

All paths below are relative to the repository root:

| Command | What it does |
| --- | --- |
| `.\build\bin\vaultheist.exe` | Full game (menu, campaign, Acts 1–4, Endless, Sandbox, leaderboard, settings). |
| `.\build\bin\tests.exe` | Full unit test suite; prints one `PASS`/`FAIL` line per case. |
| `.\build\bin\bench.exe` | Generates seven deterministic CSVs in `bench/results`. |

## How to play

### Campaign

Main Menu → **Start the Heist**. Clear a mission to unlock the next (needs ≥1
star on the previous level). Esc / Backspace returns to the menu.

### Tree levels (Acts 1–3)

- **Clicking the tree:** rooms/nodes are played by clicking them.
  Traversal levels expect clicks in the traversal order (pre, in, post or
  level). Search levels (2.x) navigate left or right at each room to find the
  target; the alarm counts comparisons. Act 2's 2.4 is a range query `[lo, hi]`.
- **Token placement (1.2, 1.4–1.5):** drag room tokens into tree slots.
- **Enter:** type digits, then Enter submits the numeric answer.
- **Hint:** `H` (caps stars at two). **Undo:** `Z`. **Reset:** `R`.
- **Space / Left / Right** play/pause and step event animation.
- **Esc:** back to the Campaign Map.
- **3.x:** balance badges; manual LL/RR/LR/RL repair; 3.4 auto-repair; 3.5
  order-statistic heist (`kth` / subtree sizes).

### Act 4 (Heap Fortress)

Synchronised heap tree + array view. Swap by selecting two indices; extract by
selecting root and last active, then restore with parent–child swaps. `R`
reset, Space / arrows for animation, Enter to continue when complete.

### Endless

Seeded stream of mixed inserts and deletes on an AVL. The run ends when the
validator rejects the tree, or after 25 operations. Type a value and press
**Enter** to insert. Survival time is posted to the local leaderboard as
`Endless`.

### Sandbox

Free-form toolbox. `Structure` cycles **Binary Tree → BST → AVL → Max Heap**.
Trees: Insert / Delete / Search / Range / Rotate. Heap: Insert / Extract /
Build Max Heap.

## Controls (`vaultheist`)

- **Mouse:** left-click navigation and level interactions.
- **Esc:** back out (menu / campaign). Closing the window quits.
- **Settings:** animation speed and comfort options.

## Folder map

| Path | Contents |
| --- | --- |
| `src/model` | GUI-free nodes, events, trees, MaxHeap, validators. |
| `src/util` | `DynArray`, `Stack`, `Queue`, `Rng`, `NodeId`. |
| `src/game` | Levels, scoring, Acts 1–4 logic, TreeLayout / TreeMotion. |
| `src/gui` | Screens, AnimationController, Renderer, TreeAnimationView, HeapView. |
| `tests` | Unit suites; `tests/TEST_REPORT.md` records a full run. |
| `data/levels` | `act1.json`..`act4.json`; see `docs/level-guide.md`. |
| `docs` | Project report, level format/guide, bench notes. |
| `screenshots` | Report figures (GUI, AVL rotations, Act 4 heap). |
| `bench` | Deterministic BST / AVL / heap benchmarks. |
| `assets/fonts` | `Inter-Regular.ttf` and its licence. |

## Tests and benches

```powershell
.\build\bin\tests.exe
.\build\bin\bench.exe
```

`bench.exe` writes CSVs under `bench/results/`; see `docs/bench-notes.md`.

## Documentation

| File | Purpose |
| --- | --- |
| `docs/Project_Report.md` / `.html` | Formal written report for submission. |
| `docs/level-format.md` | Level JSON reference. |
| `docs/level-guide.md` | How to add a level. |
| `docs/bench-notes.md` | Benchmark methodology. |
