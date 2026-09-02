#include "simulation.h"

#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    WORLD_WIDTH = 32,
    WORLD_HEIGHT = 12,
    INITIAL_PLANTS = 45,
    INITIAL_GRAZERS = 7,
    COMMAND_LIMIT = 10000
};

static void print_help(FILE *output)
{
    fputs(
        "Commands:\n"
        "  <enter> or step [N]       advance the ecosystem\n"
        "  spawn plant|grazer [N]    add organisms\n"
        "  show                       redraw the world\n"
        "  stats                      print population totals\n"
        "  help                       show these commands\n"
        "  quit                       leave the simulation\n",
        output
    );
}

static bool parse_count(const char *text, size_t default_value, size_t *result)
{
    char *end;
    unsigned long value;

    if (text == NULL) {
        *result = default_value;
        return true;
    }

    errno = 0;
    value = strtoul(text, &end, 10);
    if (errno != 0 || *text == '\0' || *end != '\0' ||
        value > COMMAND_LIMIT) {
        return false;
    }
    *result = (size_t)value;
    return true;
}

static bool parse_seed(const char *text, uint32_t *result)
{
    char *end;
    unsigned long value;

    errno = 0;
    value = strtoul(text, &end, 10);
    if (errno != 0 || *text == '\0' || *end != '\0' || value > UINT32_MAX) {
        return false;
    }
    *result = (uint32_t)value;
    return true;
}

static void print_stats(Simulation *simulation)
{
    SimulationStats stats = simulation_stats(simulation);
    printf(
        "tick=%llu plants=%zu grazers=%zu entities=%zu\n",
        (unsigned long long)stats.tick,
        stats.plants,
        stats.grazers,
        stats.entities
    );
}

static int interactive_loop(Simulation *simulation)
{
    char line[128];

    puts("Tiny Ecosystem — an ECS text simulation");
    print_help(stdout);
    simulation_render(simulation, stdout);

    for (;;) {
        char *command;
        char *argument;
        char *count_text;
        size_t count;

        fputs("sim> ", stdout);
        fflush(stdout);
        if (fgets(line, sizeof(line), stdin) == NULL) {
            fputc('\n', stdout);
            return 0;
        }

        command = strtok(line, " \t\r\n");
        if (command == NULL) {
            simulation_step(simulation, 1u);
            simulation_render(simulation, stdout);
        } else if (strcmp(command, "step") == 0 || strcmp(command, ".") == 0) {
            if (!parse_count(strtok(NULL, " \t\r\n"), 1u, &count)) {
                puts("step count must be between 0 and 10000");
                continue;
            }
            simulation_step(simulation, count);
            simulation_render(simulation, stdout);
        } else if (strcmp(command, "spawn") == 0) {
            SimulationSpecies species;
            size_t spawned;

            argument = strtok(NULL, " \t\r\n");
            count_text = strtok(NULL, " \t\r\n");
            if (argument == NULL || !parse_count(count_text, 1u, &count)) {
                puts("usage: spawn plant|grazer [count]");
                continue;
            }
            if (strcmp(argument, "plant") == 0) {
                species = SIMULATION_PLANT;
            } else if (strcmp(argument, "grazer") == 0) {
                species = SIMULATION_GRAZER;
            } else {
                puts("species must be plant or grazer");
                continue;
            }
            spawned = simulation_spawn(simulation, species, count);
            printf("spawned %zu of %zu requested\n", spawned, count);
            simulation_render(simulation, stdout);
        } else if (strcmp(command, "show") == 0) {
            simulation_render(simulation, stdout);
        } else if (strcmp(command, "stats") == 0) {
            print_stats(simulation);
        } else if (strcmp(command, "help") == 0) {
            print_help(stdout);
        } else if (strcmp(command, "quit") == 0 || strcmp(command, "q") == 0) {
            return 0;
        } else {
            printf("unknown command: %s (try 'help')\n", command);
        }
    }
}

static void print_usage(const char *program)
{
    printf(
        "Usage: %s [--seed N] [--steps N]\n"
        "Without --steps, starts the interactive text simulation.\n",
        program
    );
}

int main(int argc, char **argv)
{
    uint32_t seed = UINT32_C(0xc0ffee);
    size_t steps = 0u;
    bool batch_mode = false;
    int index;
    Simulation *simulation;
    int result;

    for (index = 1; index < argc; ++index) {
        size_t parsed;

        if (strcmp(argv[index], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        }
        if ((strcmp(argv[index], "--seed") == 0 ||
             strcmp(argv[index], "--steps") == 0) && index + 1 < argc) {
            if (strcmp(argv[index], "--seed") == 0) {
                if (!parse_seed(argv[index + 1], &seed)) {
                    fprintf(stderr, "invalid value for --seed\n");
                    return 2;
                }
            } else {
                if (!parse_count(argv[index + 1], 0u, &parsed)) {
                    fprintf(stderr, "invalid value for --steps\n");
                    return 2;
                }
                steps = parsed;
                batch_mode = true;
            }
            index++;
        } else {
            print_usage(argv[0]);
            return 2;
        }
    }

    simulation = simulation_create(WORLD_WIDTH, WORLD_HEIGHT, seed);
    if (simulation == NULL) {
        fputs("failed to create the simulation\n", stderr);
        return 1;
    }
    if (!simulation_populate(simulation, INITIAL_PLANTS, INITIAL_GRAZERS)) {
        fputs("failed to populate the simulation\n", stderr);
        simulation_destroy(simulation);
        return 1;
    }

    if (batch_mode) {
        simulation_step(simulation, steps);
        simulation_render(simulation, stdout);
        print_stats(simulation);
        result = 0;
    } else {
        result = interactive_loop(simulation);
    }

    simulation_destroy(simulation);
    return result;
}
