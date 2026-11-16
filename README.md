# Vault Heist: The Living Vault
Data Structures project, Anas and Arslan.
Stack: C++17, SFML 3.0.0 (fetched by CMake), CMake, MinGW-W64 GCC 14.

## Build and run

Run these commands from the `vault-heist` folder:

```text
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
build/bin/vaultheist.exe
```

SFML is downloaded by CMake during the first configuration. The executable
needs the copied `build/bin/data` folder beside it. The CMake build performs
that copy automatically.

In VS Code, use **Run and Debug > Run Vault Heist**. The single-file
**Compile & Run** button cannot build this multi-file CMake project.

## Current controls

- Mouse: click screen navigation buttons.
- Esc: return to Main Menu.
- Level Play demo: Space plays or pauses; Left/Right steps; Up/Down changes
  speed; R restarts; I completes instantly.

## Current structure

- `src/model`: GUI-independent nodes and event log.
- `src/gui/core`: theme, screen manager and animation playback.
- `src/gui/screens`: temporary screens and the animation demonstration.
- `tests`: model and animation checks.
- `docs`: implementation notes.

The screens are placeholders for later gameplay. Tree and heap operations will
produce EventLogs; AnimationController will play those events for the renderer.
