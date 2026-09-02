# ECS Text Simulation

A small, dependency-free C11 game scaffold built around a reusable entity
component system. The included game is a turn-based terminal ecosystem:
plants grow, grazers seek food, spend energy, reproduce, and eventually die.

The simulation is deliberately simple. It is a working slice for adding new
components and systems without tying the project to a rendering framework.

## Build and run

Requirements: a C11 compiler and CMake 3.16 or newer.

```sh
cmake -S . -B build
cmake --build build
./build/c_game
```

Press Enter to advance one tick, or use `help` to list commands. A noninteractive
run is useful for smoke tests:

```sh
./build/c_game --seed 42 --steps 100
```

## Test

```sh
ctest --test-dir build --output-on-failure
```

For a checked development build:

```sh
cmake -S . -B build-sanitize \
  -DC_GAME_WARNINGS_AS_ERRORS=ON \
  -DC_GAME_ENABLE_SANITIZERS=ON
cmake --build build-sanitize
ctest --test-dir build-sanitize --output-on-failure
```

## Structure

- `include/ecs.h`, `src/ecs.c` — generation-safe entities, up to 64 registered
  component types, component storage, and mask-based queries.
- `include/simulation.h`, `src/simulation.c` — game components and systems.
- `src/main.c` — terminal command loop and batch-mode entry point.
- `tests/` — focused ECS lifecycle/query tests and a simulation smoke test.

Components contain state only. Behavior lives in systems that query for the
component combinations they need. Destroyed entity slots are recycled, while
generation counters keep stale handles invalid.
