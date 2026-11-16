# Task 0.7: animation playback

`AnimationController` owns a copy of an EventLog and reads it in sequence. The
log acts as its event queue while retaining earlier events for backwards steps.
No SFML headers or model mutations are involved in the controller.

- `completed`: how many events have finished.
- `ticks`: progress through the current event (0 through 59).
- `accumulated`: time waiting to be processed as fixed 1/60-second ticks.
- `speed`: playback multiplier, between 0.25 and 4.
- `playing`: whether updates advance the cursor.

Each event takes 60 ticks. `ease(t)` uses `t*t*(3-2*t)` to slow down movement
near its endpoints. `load` starts paused, `play` resumes, `pause` freezes,
and `restart` resets the visuals and cursor while preserving speed.

The drawing code implements `AnimationListener`. `reset()` restores the visual
snapshot from before the operation. `onEvent(event, progress)` draws an event
at a progress between 0 and 1. It must calculate from the event's starting state,
not repeatedly add a change on every tick. It must never alter the live model.
The listener must outlive its registration with the controller.

Right completes the current event and pauses. Left returns to the current
event's start if it is partly animated; otherwise it removes one completed
event. Rewinding resets the visual snapshot and reapplies the completed prefix.
Instant playback performs the same reconstruction through the entire log,
without intermediate animation. This is visual playback, not gameplay undo.

The Level Play demonstration uses five dummy events and progress bars.
Navigate Menu > Campaign > Briefing > Start Level, then use:

- Space: play/pause
- Left/Right: step backwards/forwards
- Up/Down: double/halve speed
- R: restart
- I: finish instantly
- Esc: menu

Only the active screen updates, so leaving Level Play freezes its demo.
Returning resumes its stored playback state. Restart with R when needed.

Headless tests cover timing, pause/resume, stepping, replay reset, speed,
instant completion, ownership, empty logs and frame partitioning. Manual visual
verification remains required. Task 0.7 is integrated into `develop`, but its
tracker box stays unticked until `develop` is merged into `main`.
