# Task 0.6: app shell

The app shell separates the main loop from individual screens.

- `main.cpp` creates the window, theme and screen manager. Each frame it sends
  input to the manager, updates the active screen and asks it to draw.
- `Screen` defines the common input, update and draw functions.
- `ScreenManager` owns all nine screen objects and stores the active screen id.
- `StubScreen` supplies the temporary title, description and navigation buttons.
- `Theme` owns the shared font and uses `Palette` from `Style.h` to create
  consistent labels, panels and buttons.

Screen ids are class constants because project rules forbid enums. A fixed
array stores the screens because STL containers are forbidden. Screens do not
call the model. Esc is handled once by ScreenManager and returns to the menu.

The navigation paths are:

```text
Menu -> Campaign -> Briefing -> Level Play -> Result
Menu -> Endless -> Leaderboard
Menu -> Sandbox
Menu -> Settings
```

The app shell uses the shared Inter font at `assets/fonts/Inter-Regular.ttf`.
Every label converts UTF-8 text through `sf::String::fromUtf8` before giving it
to SFML, so symbols remain correct.

The placeholder navigation was manually checked on Arslan's laptop. Task 0.6
was merged into `develop` after adapting it to the shared renderer style.
