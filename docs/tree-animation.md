# Tree animations (task A6)

`vh::TreeAnimationView` (`src/gui/tree/`) implements Arslan's
`AnimationListener` without changing its public signatures. `AnimationController`
calls `reset()` and `onEvent(event, progress)`; the view stores only the active
event, its progress and a replay counter. It never stores a node position.

`TreeMotion` (`src/game/tree/`, part of `gamecore`) holds the pure float
interpolation. A script is built before playback:
`TreeMotion::simple(layoutBefore, events, layoutAfter)`. For a rotation the
post-rotation layout comes from `TreeLayout::compute(tree)` on the already
rebalanced model, so it exists before the first animation frame.

Every draw asks `TreeMotion::frameAt(activeIndex, progress)` for an absolute
frame. `frameAt` returns:

- the recorded layout before the active event for non-positional events;
- an interpolation between the previous and current recorded layout for
  `ROTATE`, `INSERT`, `UNLINK` and `SWAP_VALUE`;
- the final layout once the event index is past the end.

`AnimationController` rebuilds by resetting and replaying every completed event
at progress 1. Skip-to-end (`finishInstantly`), step-back/undo and pause all go
through the same rebuild, so every node ends exactly on a `TreeLayout` position.
No interpolation is incremental and no stale ids survive.

## Event handlers

| Event | Motion / cue |
| --- | --- |
| `VISIT` | node state VISITED with the `VISIT` tag |
| `COMPARE` | node state WARNING, `CMP <` / `CMP >` text |
| `INSERT` | new node at its layout position, `NEW` badge |
| `UNLINK` | SKIPPED tag, edge retracts toward the parent, `OUT` badge |
| `SWAP_VALUE` | old and new values slide between the two nodes, `SWAP` label |
| `ROTATE` | tween to the recorded post-rotation layout, LL/RR/LR/RL badge |
| `UPDATE_HEIGHT` / `UPDATE_SIZE` | `H` / `S` badge flicker |
| `BALANCE_ALERT` | VIOLATION state, white outline and `BAD n` badge |
| `HIGHLIGHT_SUBTREE` | subtree tinted with `VISIT`, `SKIP` or `OK` tag |

All colours come from `Palette` and `NodeState`; every state also has a text tag
or badge, so colour is never the only signal.

## Demo

`treeanimdemo` (target only in `CMakeLists.txt`) opens a window with the keys:

- `1`–`4`: real AVL inserts that trigger LL, RR, LR and RL
- `I` insert, `D` delete, `Space` play/pause, `Left`/`Right` step, `P` screenshot, `Esc` quit
- `--smoke`: open, play one scenario, save `screenshots/tree_anim.png`, close
