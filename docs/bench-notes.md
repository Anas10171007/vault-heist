# Benchmark notes

Run `build/bin/bench.exe` from the repository root. Results are written to
`bench/results/`; an alternative output directory can be passed as the first
argument.

The suite uses fixed input sizes and deterministic RNG seeds. Event recording is
disabled so animation-log allocation does not distort model timings. Each CSV
also records operation counters or final height, because short wall-clock runs
can vary between machines.

Seven files are produced: sorted and random insertion for BST and AVL, Max Heap
build, Heap Sort, and a hand-written insertion-versus-selection sort baseline.
No standard-library sorting algorithm is used.

Timings are single-process microsecond measurements. They are suitable for trend
charts, not for comparing different computers. Pause background applications
and run the executable more than once when stable absolute timing is needed.
