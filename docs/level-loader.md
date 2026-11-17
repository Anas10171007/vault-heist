# Task 0.10 level loader

`JsonReader` is a small character-by-character reader for the limited JSON in
`docs/level-format.md`. It understands objects, arrays, strings, whole numbers
and booleans. `skipValue` lets newer files contain unknown fields without
breaking this version of the game.

`LevelManager` reads the four act files into `DynArray<LevelDef>`. It checks the
file version and act number, required fields, unique IDs, valid structure/task
pairs, and the required data for explicit or generated levels. `load(id)`
returns the parsed definition or `nullptr` when the ID is unknown.

The future tree and heap models will use the returned `LevelDef` to construct
the actual structure and calculate `optimalSteps`. That part is deliberately
not faked while those model interfaces do not exist.
