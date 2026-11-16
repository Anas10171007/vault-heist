# Task 0.6: app shell

The app shell separates the main loop from individual screens.

- `main.cpp` creates the window, theme and screen manager. Each frame it sends
  input to the manager, updates the active screen and asks it to draw.
- `Screen` defines the common input, update and draw functions.
- `ScreenManager` owns all nine screen objects and stores the active screen id.
- `StubScreen` supplies the temporary title, description and navigation buttons.
- `Theme` owns the shared font and colours and creates consistent labels, panels
  and buttons.

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

The included Tuffy font is public domain (CC0). CMake copies the `data` folder
beside the executable after every game build.

The placeholder navigation was manually checked on Arslan's laptop. Task 0.6
must remain unticked in the shared tracker until its branch is merged.
