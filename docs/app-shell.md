# Task 0.6: app shell

The app shell separates the main loop from individual screens.

- `main.cpp` creates the window, theme and screen manager. Each frame it sends
  input to the manager, updates the active screen and asks it to draw.
- `Screen` defines the common input, update and draw functions.
- `ScreenManager` owns all nine screen objects and stores the active screen id.
- `StubScreen` supplies temporary title/description screens (Endless is still a
  stub). Sandbox and Leaderboard are real screens (`SandboxScreen`,
  `LeaderboardScreen`).
- `Theme` owns the shared font and uses `Palette` from `Style.h` to create
  consistent labels, panels and buttons.
- On construction, `ScreenManager` calls `SaveManager::loadProgress(session)` so
  campaign stars survive relaunch (B11). Finishing a level also saves progress
  and posts a local leaderboard score from `LevelScreen::storeResult`.

Screen ids are class constants because project rules forbid enums. A fixed
array stores the screens because STL containers are forbidden. Esc is handled
once by ScreenManager and returns to the menu.

The navigation paths are:

```text
Menu -> Campaign -> Briefing -> Level Play -> Result
Menu -> Endless -> Leaderboard   (Endless still stub)
Menu -> Sandbox                  (Binary / BST / AVL / Max Heap playground)
Menu -> Leaderboard              (local VHLEAD1 top scores)
Menu -> Settings
```

Fonts come from the vault-noir theme (system Bahnschrift / Cascadia fallbacks
under `assets/fonts/`). Task 0.6 was merged into `develop` after adapting it to
the shared renderer style.
