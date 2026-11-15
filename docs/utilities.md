# Utility containers and seeded RNG (task 0.3)

The utilities are in `src/util/` and use the same `vh` namespace as the model
classes. They do not use STL containers, STL algorithms, enums, or global
variables.

```cpp
#include "util/DynArray.h"
#include "util/Stack.h"
#include "util/Queue.h"
#include "util/Rng.h"

using namespace vh;

DynArray<int> values;
values.push(10);
values.push(20);

Stack<int> stack;
stack.push(5);
int last = stack.pop();

Queue<int> queue;
queue.enqueue(7);
int first = queue.dequeue();

Rng rng(123u);
rng.shuffle(values);
```

## Containers

- `DynArray<T>` stores a pointer, a count, and a capacity. When it becomes full,
  it creates an array with twice the capacity, copies the values, deletes the old
  array, and continues using the new one. It supports `push`, `pop`, `at`,
  `operator[]`, `size`, `empty`, and `clear`, plus independent copying.
- `Stack<T>` uses a `DynArray`. It adds and removes values at the end, following
  last-in-first-out order.
- `Queue<T>` uses two custom stacks. New values enter the incoming stack. When
  the outgoing stack is empty, values move into it in reverse order, placing the
  oldest value on top. This gives first-in-first-out order.

`pop` and `dequeue` return a default value such as `0` when empty. Before using
`top`, `front`, `at`, or `operator[]`, the caller must check that the container
is not empty and that the index is valid.

## Reproducible randomness

`Rng` stores an `unsigned int` state. `next()` updates it with:

```text
state = state * 1664525 + 1013904223
```

Unsigned integer wraparound keeps the state within the supported 32-bit range
on the project compiler. The same seed and the same order of calls produce the
same sequence. Seed 1 begins:

```text
1015568748 1586005467 2165703038 3027450565 217083232
```

`range(lo, hi)` uses the remainder operator to produce a value inside the
inclusive range and swaps the bounds if they arrive in reverse order. Project
code should use normal level-sized ranges. `shuffle` uses the Fisher-Yates
algorithm: it walks backward through the array and swaps each value with a
random value at or before its position.

This generator exists for repeatable game levels, not for security.

## Verification

`tests/util/utility_tests.cpp` checks array growth and copying, stack and queue
order, reuse, fixed RNG output, ranges, and shuffle preservation.

```text
cmake --build build --target tests
build/bin/tests.exe
```

Task 0.3 remains unticked until it is merged into the shared branch and the
fixed seed is checked on the second laptop.
