#ifndef C_GAME_SIMULATION_H
#define C_GAME_SIMULATION_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

typedef struct Simulation Simulation;

typedef struct {
    uint64_t tick;
    size_t plants;
    size_t grazers;
    size_t entities;
} SimulationStats;

typedef enum {
    SIMULATION_PLANT,
    SIMULATION_GRAZER
} SimulationSpecies;

Simulation *simulation_create(int width, int height, uint32_t seed);
void simulation_destroy(Simulation *simulation);

bool simulation_populate(Simulation *simulation, size_t plants, size_t grazers);
size_t simulation_spawn(
    Simulation *simulation,
    SimulationSpecies species,
    size_t count
);
void simulation_step(Simulation *simulation, size_t steps);

SimulationStats simulation_stats(Simulation *simulation);
void simulation_render(Simulation *simulation, FILE *output);

#endif

