# Playable Acts 1-3

`LevelPlayScreen` drives any `LevelPlay` object from `src/game/act1`..`act3`.
It draws the current tree through `TreeLayout`, `TreeMotion` and
`TreeAnimationView`, uses `Camera` for wheel zoom / drag pan / fit-to-view, and
shows the mission, `AlarmMeter`, steps versus optimal, score, penalties, an
event log and a hint line. Every state has a text tag or badge, so colour is
never the only signal.

## Controls

- **Mouse:** click tree nodes for traversals, search, badges, LCA, mirror and
  rotation pivots; click tokens then slots for reconstruction; click case
  buttons for delete-case and rotation-case tasks; Confirm / Auto / numeric
  answer buttons are shown as needed.
- **Toolbar:** `UNDO Z`, `RESET`, `HINT H`, `SPEED` (0.5x / 1x / 2x), `MAP`.
- **Keys:** `Space` play/pause, `Left`/`Right` step the event replay, `Z` undo,
  `H` hint, digits and `Backspace`/`Enter` for numeric answers, `Esc` back to
  the campaign map.
- **Win/fail overlay:** stars, steps versus optimal, penalties, `RETRY`,
  `NEXT` (when another level exists) and `MAP`.

Undo uses the exact `LevelPlay` snapshot added in part 1: the tree keeps its
node ids, so a rotation can be undone and animated from the restored shape.

## Campaign map

`CampaignMapScreen` lists all 15 Act 1-3 levels from `LevelManager`, with
session stars and all levels unlocked. Act 4 is shown with `COMING SOON` and is
not clickable. Clicking a level loads it into `LevelPlayScreen`; `Esc` returns
to the main menu.

## ScreenManager registration

The only change to Arslan's `ScreenManager.cpp` was:

- include `CampaignMapScreen.h` and `LevelPlayScreen.h`;
- replace the Campaign Map `StubScreen` with `CampaignMapScreen(theme, play, play->session())`;
- replace the `HeapLevelScreen` at `ScreenId::LEVEL_PLAY` with the same
  `LevelPlayScreen* play`;
- make `Esc` from `LEVEL_PLAY` return to `CAMPAIGN_MAP` instead of the menu.

`AnimationDemoScreen` was not registered before and is still not registered.
All other stub screens are unchanged.

## Smoke check

`levelsmoke.exe` plays every one of the 15 levels to a win through
`LevelPlayScreen::playAllLevels` (window-free, stars checked), then renders the
campaign map and one level of each act to:

- `screenshots/campaign_map.png`
- `screenshots/act1_level.png`
- `screenshots/act2_level.png`
- `screenshots/act3_level.png`

`levelsmoke.exe --logic` runs only the logic check.
