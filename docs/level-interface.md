# Level play interface (tasks A7-A9)

There was no common level interface in the repository. This is the smallest one
the game needs. It is pure C++ in `gamecore` (`src/game/LevelPlay.h`); it has no
SFML, no enums and no global state.

```cpp
LevelPlay* createLevelPlay(const LevelDef& definition);  // game/LevelPlayFactory.h
```

`createLevelPlay` returns `nullptr` for an unknown task. The returned object is
owned by the caller and exposes:

| Call | Purpose |
| --- | --- |
| `bool load(const LevelDef&)` | Build the model tree from `data`, compute `optimalSteps()` from the model, reset scoring. |
| `void reset()` | Restart the current level with the same seed and definition. |
| `PlayResult apply(const PlayerAction&)` | Submit one player action. |
| `PlayResult hint()` | Charge a `HINT` penalty and return the next correct action in `hintAction`. |
| `PlayerAction nextAction() const` | The correct next step, with **no** penalty. Use it for tutorial highlighting and tests. |
| `int optimalSteps() const` | Computed from the model, never read from the level file. |
| `bool isComplete()`, `bool isFailed()` | Win and lose state. |
| `const ScoreSystem& score() const` | Steps, alarm, capacity, stars, score. |
| `const BinaryTree& tree() const` | Current tree for drawing. |
| `const LevelDef& definition() const` | Title, mission and rules. |

`PlayResult` contains `correct`, `wrong`, `won`, `lost`, a message, an `EventLog`
of model events, and an optional hint action.

## Exactly what the screens must call

1. `LevelManager manager; manager.loadAll("data/levels");`
2. `const LevelDef* def = manager.load(id);` where `id` is e.g. `"1.3"`.
3. `LevelPlay* play = createLevelPlay(*def);`
4. `play->load(*def);` once when the level screen opens, `play->reset()` for Retry.
5. Feed clicks into `play->apply(action)`; use the action builders in
   `LevelPlay.h`: `clickNode(id)`, `placeToken(slot, value)`, `answer(value)`,
   `chooseCase(choice)`, `confirmAction()`, `autoAction()`.
6. Animate `result.events` (A6), then show `play->score()` and the tree from
   `play->tree()`.
7. On `result.won` or `play->isFailed()`, show the result screen and `delete play`.

All optimal-step counts and the alarm capacity come from the model and
`ScoreSystem`. `LevelDef` carries no tuned numbers. Wrong traversal-style
clicks charge `PenaltyKind::WRONG_TRAVERSAL` (+1 alarm); wrong rotations charge
`PenaltyKind::WRONG_ROTATION` (+2 alarm, -50 points); a hint charges
`PenaltyKind::HINT` and caps the result at two stars.

## Act-specific notes

- **Act 1:** `answer(value)` is used for the height/diameter and LCA/path
  questions. `AUTO` on a mirror level performs every remaining swap but records
  a hint, so it cannot earn three stars.
- **Act 2:** `chooseCase(DeleteCase::...)` selects the delete case before
  `confirmAction()` performs the delete. `bst_boss` recomputes the alarm
  capacity from the remaining model steps after each checkpoint, so the
  capacity shrinks and the per-stage score history is cleared.
- **Act 3:** single rotations need only `clickNode(pivot)`. Double rotations
  need `chooseCase(RotationType::LR/RL)` and then `clickNode(pivot)`. Auto
  prediction reads the `extra` field of `ROTATE` events from a serialized clone
  with the candidate key inserted; `-1` means no rotation. The kth boss uses
  `kth(n - k + 1)`, an answer, then a repair rotation and a `size()` check
  before the next stage.
