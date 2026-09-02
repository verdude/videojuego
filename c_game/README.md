# ECS Text Simulation

A small C11 game scaffold built with the [Flecs](https://www.flecs.dev/flecs/)
entity component system. The included game is a turn-based terminal ecosystem:
plants grow, grazers seek food, spend energy, reproduce, and eventually die.

The simulation is deliberately simple. It is a working slice for adding new
components and systems without tying the project to a rendering framework.

## Build and run

Requirements: a C11 compiler, Git, and CMake 3.16 or newer. The first configure
downloads the pinned Flecs 4.1.5 release through CMake's `FetchContent`.

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

- `CMakeLists.txt` — downloads and links the pinned Flecs static library.
- `include/simulation.h`, `src/simulation.c` — Flecs components, queries, and
  game systems.
- `src/main.c` — terminal command loop and batch-mode entry point.
- `tests/` — simulation smoke tests.

Components contain state only. Behavior lives in systems that query for the
component combinations they need. Flecs supplies archetype storage, query
iteration, entity recycling, and generation-safe handles.
