#include "simulation.h"

#include <assert.h>
#include <stdio.h>

int main(void)
{
    Simulation *simulation = simulation_create(20, 10, 12345u);
    SimulationStats stats;

    assert(simulation != NULL);
    assert(simulation_populate(simulation, 30u, 4u));

    stats = simulation_stats(simulation);
    assert(stats.tick == 0u);
    assert(stats.plants == 30u);
    assert(stats.grazers == 4u);
    assert(stats.entities == stats.plants + stats.grazers);

    simulation_step(simulation, 25u);
    stats = simulation_stats(simulation);
    assert(stats.tick == 25u);
    assert(stats.entities == stats.plants + stats.grazers);
    assert(stats.entities <= 512u);
    assert(simulation_spawn(simulation, (SimulationSpecies)99, 1u) == 0u);

    simulation_destroy(simulation);
    puts("simulation tests passed");
    return 0;
}
