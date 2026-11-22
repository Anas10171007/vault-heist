# LLM handoff: Arslan's project context

Read this before proposing or making changes. It records decisions made with
Arslan that are not obvious from source code alone.

## Working style

- Arslan is a third-semester student and must understand and be able to
  explain every submitted part. Teach the code as well as the idea: explain
  what each relevant line/function does before or while changing it.
- Do not silently implement a large feature. First give a small plan, explain
  the design, then make a focused change with Arslan's approval.
- Do not commit, push, edit cloud documents, or change somebody else's branch
  unless Arslan explicitly asks. Pull/fetch and inspect before any push.
- Preserve other team members' work. `develop` is Anas's integration branch.
- Use the project brief and the latest TA announcement as the authority when
  they conflict with a suggestion or an earlier assumption.

## Agreed coding rules

These are both TA rules and Arslan's agreed style unless the project brief
requires otherwise.

- No STL containers or STL algorithms: no `vector`, `list`, `map`, `stack`,
  `queue`, `sort`, `max`, `swap`, etc. Use the project's `DynArray`, `Stack`,
  `Queue`, and hand-written algorithms.
- `std::string` is explicitly allowed. SFML GUI types (`sf::Vector2f`,
  `sf::Text`, `sf::Color`, `sf::Event`, etc.) are allowed only for GUI work.
- No global variables and no C++ `enum`s. Use named `static constexpr int`
  members in a struct/class for integer categories.
- Keep `namespace vh`, because the existing model uses it. Prefer
  `using namespace std;` in implementation-facing code rather than repeated
  `std::` qualifications, matching Arslan's requested style.
- Keep code simple and readable for a third-semester student. Avoid adding
  exceptions (`throw`, `try`, `catch`), fixed-width integer types,
  `int64_t`, overly advanced templates, and unnecessary casts. Prefer `int`
  when it is sufficient.
- Do not add a library just for convenience. Explain every library that is
  genuinely necessary.
- A leaf has height **1**, and an empty tree has height **0**.
- Use `sf::String::fromUtf8` for any literal passed directly to `sf::Text` so
  symbols such as `→`, `≥`, and `✓` survive. Use `Renderer::NODE_HIT_RADIUS`
  for tree-node clicks. Reuse `Palette::...`; do not create a second colour
  system in `Theme`.

## Project architecture

Maintain the intended direction of dependencies:

`model -> EventLog -> AnimationController -> GUI renderer`

Input goes in the reverse direction through level logic to the model. The data
structures must never depend on SFML or GUI screens. Each model operation
returns events; the GUI animates those events instead of reimplementing the
algorithm.

`docs/event-payloads.md`, `docs/level-interface.md`, `docs/level-format.md`,
and `docs/score-system.md` are the local references for those contracts.

## Scoring decisions already made

- `addStep()` increases steps and alarm for a valid player operation.
- A wrong traversal/invalid heap action adds one alarm but is not a normal
  valid step.
- A wrong rotation adds two alarm and a 50-point deduction.
- The score formula is the project brief's `1000 - 10 * alarmSteps -
  pointPenalty + bonusPoints`, floored at zero.
- Use the literal two-star condition `steps <= optimalSteps * 1.25`; do not
  rewrite it into an integer multiplication merely to avoid a decimal.
- Alarm capacity is `ceil(1.5 * optimalSteps) + 2`, implemented as
  `(optimalSteps * 3 + 1) / 2 + 2`.

## Current repository status (21 November 2026)

- Current feature branch: `arslan/B4-B9-finish`.
- The branch has four commits not in `origin/develop`:
  `bad7cb2`, `f34249c`, `2abf50b`, and `bbd505c`.
- They have **not** been merged into `develop`. Do not merge them blindly.
- A clean serial build succeeds with `cmake --build build -- -j1`.
- `build/bin/tests.exe` passed all 602 current checks on this machine.
- `git diff --check origin/develop..HEAD` passes.

## Audit findings: fix before merging the feature branch

1. `src/model/heap/MaxHeap.cpp` uses `stoi` plus `try`/`catch` in snapshot
   deserialization. This directly breaks Arslan's no-exceptions agreement.
   Replace it with a small manual integer parser and tests for malformed and
   boundary input.
2. `HeapOperationLevel::undo()` restores only a heap snapshot. It must also
   restore the score/alarm/penalties/finished state, otherwise undo still
   charges the player and can leave a completed score state behind.
3. Act 4 needs distinct player-facing behaviour:
   - 4.3: repeated extract-max, with the player seeing the active heap and
     sorted region after each extraction.
   - 4.4: an actual race/operation-alarm constraint, not simply 4.3 with a
     longer array.
   - 4.5: player-performed Heap Sort steps. A one-click automatic sort is a
     demonstration, not the brief's independent final task.
4. The result screen must show Act 4's final sorted array and the
   `Final Vault Unlocked` achievement after full Heap Sort.
5. Benchmark work is incomplete against project-brief Section 10: add the
   required 10,000 tree case, search-cost and rotation experiments,
   order-statistics comparison, heap inputs (random/ordered for build;
   random/sorted/reverse/duplicate-heavy for sorting), and direct comparison
   with course sorting algorithms.
6. Synchronise documentation counts: `tests/TEST_REPORT.md` says 602 while
   `docs/report-part-a.md` says 589. The heap-results section of the test
   report is still empty.
7. The Act 1-3 GUI's current `Next Step` calls `logic->nextAction()` and
   applies it automatically. Treat it as a temporary demo path, not finished
   player interaction.

## Still to review / fix (keep current)

- **Endless mode (brief survival mode)** — still a `StubScreen` only. Brief wants
  a stream of mixed inserts/deletes, score = survival time while the tree stays
  valid, difficulty ramping the operation rate, and save of endless tree/heap +
  seed. **Not the same as Sandbox** (Sandbox = free practice; Endless = timed
  survival). Implement or confirm with TA if optional before demo.
- Commit + push uncommitted Act 4 / P2 polish when Arslan asks (extract UX,
  timeline scrubber, UI declutter, solo extract, result Replay).
- Manual playthrough of 4.1–4.5 (complete, fail, retry, undo, Results → Replay).
- Merge with Anas `develop` when ready (do not force-merge alone).
- Written report PDF + GUI/heap/AVL screenshots; refresh `TEST_REPORT.md` counts.
- Fold Section 10 bench CSV numbers into the report.

## Verification routine

After a focused change:

1. Read the changed code with Arslan and explain it in plain English.
2. Run `git diff --check`.
3. Configure after new files/CMake changes:
   `cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug`.
4. Build serially if memory is tight: `cmake --build build -- -j1`.
5. Run `./build/bin/tests.exe` and update test documentation only from the
   real output.
6. Manually test the relevant GUI flow before claiming it works.

## Build and tool notes

- This is C++17, CMake, SFML 3.0.0, MinGW-w64 GCC 14 on Windows.
- Build/run commands are in `README.md`. The VS Code single-file Compile &
  Run button cannot build this multi-file CMake project; use the configured
  CMake/debug target instead.
- `data/levels/act4.json` must keep the task names: `heap_build`,
  `heap_repair`, `heap_extract`, `heap_race`, `heap_sort`.
- The level JSON schema is in `docs/level-format.md`; do not invent fields
  without updating its parser/documentation/tests.

