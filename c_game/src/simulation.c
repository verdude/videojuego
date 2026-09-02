#include "simulation.h"

#include "ecs.h"

#include <inttypes.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

enum {
    SIMULATION_CAPACITY = 512,
    PLANT_NUTRITION = 7,
    GRAZER_STARTING_ENERGY = 14,
    GRAZER_REPRODUCTION_ENERGY = 24,
    GRAZER_MAX_AGE = 90
};

typedef struct {
    int x;
    int y;
} Position;

typedef struct {
    char glyph;
} Appearance;

typedef struct {
    int nutrition;
} Plant;

typedef struct {
    int energy;
    int age;
} Grazer;

struct Simulation {
    EcsWorld *world;
    EcsComponent position;
    EcsComponent appearance;
    EcsComponent plant;
    EcsComponent grazer;
    EcsEntity *actor_snapshot;
    char *grid;
    int width;
    int height;
    uint64_t tick;
    uint32_t random_state;
};

static uint32_t random_next(Simulation *simulation)
{
    uint32_t value = simulation->random_state;

    value ^= value << 13u;
    value ^= value >> 17u;
    value ^= value << 5u;
    simulation->random_state = value;
    return value;
}

static int random_coordinate(Simulation *simulation, int limit)
{
    return (int)(random_next(simulation) % (uint32_t)limit);
}

static uint64_t component_pair(EcsComponent first, EcsComponent second)
{
    return ecs_component_mask(first) | ecs_component_mask(second);
}

static EcsEntity entity_at(
    Simulation *simulation,
    int x,
    int y,
    EcsComponent kind
)
{
    EcsIterator iterator = ecs_query(
        simulation->world,
        component_pair(simulation->position, kind)
    );
    EcsEntity entity;

    while (ecs_query_next(&iterator, &entity)) {
        const Position *position = ecs_component_get_const(
            simulation->world,
            entity,
            simulation->position
        );
        if (position->x == x && position->y == y) {
            return entity;
        }
    }
    return ECS_ENTITY_INVALID;
}

static bool cell_is_open(Simulation *simulation, int x, int y)
{
    EcsIterator iterator = ecs_query(
        simulation->world,
        ecs_component_mask(simulation->position)
    );
    EcsEntity entity;

    while (ecs_query_next(&iterator, &entity)) {
        const Position *position = ecs_component_get_const(
            simulation->world,
            entity,
            simulation->position
        );
        if (position->x == x && position->y == y) {
            return false;
        }
    }
    return true;
}

static bool find_open_cell(Simulation *simulation, int *x, int *y)
{
    size_t attempt;
    size_t attempts = (size_t)simulation->width * (size_t)simulation->height * 2u;

    for (attempt = 0u; attempt < attempts; ++attempt) {
        int candidate_x = random_coordinate(simulation, simulation->width);
        int candidate_y = random_coordinate(simulation, simulation->height);
        if (cell_is_open(simulation, candidate_x, candidate_y)) {
            *x = candidate_x;
            *y = candidate_y;
            return true;
        }
    }
    return false;
}

static EcsEntity spawn_plant_at(Simulation *simulation, int x, int y)
{
    EcsEntity entity = ECS_ENTITY_INVALID;
    Position *position;
    Appearance *appearance;
    Plant *plant;

    if (!cell_is_open(simulation, x, y)) {
        return entity;
    }

    entity = ecs_entity_create(simulation->world);
    if (!ecs_entity_is_alive(simulation->world, entity)) {
        return ECS_ENTITY_INVALID;
    }

    position = ecs_component_add(simulation->world, entity, simulation->position);
    appearance = ecs_component_add(simulation->world, entity, simulation->appearance);
    plant = ecs_component_add(simulation->world, entity, simulation->plant);
    if (position == NULL || appearance == NULL || plant == NULL) {
        ecs_entity_destroy(simulation->world, entity);
        return ECS_ENTITY_INVALID;
    }

    position->x = x;
    position->y = y;
    appearance->glyph = '*';
    plant->nutrition = PLANT_NUTRITION;
    return entity;
}

static EcsEntity spawn_grazer_at(
    Simulation *simulation,
    int x,
    int y,
    int starting_energy
)
{
    EcsEntity entity = ECS_ENTITY_INVALID;
    Position *position;
    Appearance *appearance;
    Grazer *grazer;

    if (!cell_is_open(simulation, x, y)) {
        return entity;
    }

    entity = ecs_entity_create(simulation->world);
    if (!ecs_entity_is_alive(simulation->world, entity)) {
        return ECS_ENTITY_INVALID;
    }

    position = ecs_component_add(simulation->world, entity, simulation->position);
    appearance = ecs_component_add(simulation->world, entity, simulation->appearance);
    grazer = ecs_component_add(simulation->world, entity, simulation->grazer);
    if (position == NULL || appearance == NULL || grazer == NULL) {
        ecs_entity_destroy(simulation->world, entity);
        return ECS_ENTITY_INVALID;
    }

    position->x = x;
    position->y = y;
    appearance->glyph = 'g';
    grazer->energy = starting_energy;
    grazer->age = 0;
    return entity;
}

static EcsEntity nearest_plant(
    Simulation *simulation,
    const Position *origin,
    int *distance_x,
    int *distance_y
)
{
    EcsIterator iterator = ecs_query(
        simulation->world,
        component_pair(simulation->position, simulation->plant)
    );
    EcsEntity entity;
    EcsEntity nearest = ECS_ENTITY_INVALID;
    int nearest_distance = INT_MAX;

    while (ecs_query_next(&iterator, &entity)) {
        const Position *position = ecs_component_get_const(
            simulation->world,
            entity,
            simulation->position
        );
        int dx = position->x - origin->x;
        int dy = position->y - origin->y;
        int distance = abs(dx) + abs(dy);

        if (distance < nearest_distance) {
            nearest = entity;
            nearest_distance = distance;
            *distance_x = dx;
            *distance_y = dy;
        }
    }
    return nearest;
}

static int sign_of(int value)
{
    return (value > 0) - (value < 0);
}

static void move_grazer(Simulation *simulation, Position *position)
{
    int dx = 0;
    int dy = 0;
    EcsEntity food = nearest_plant(simulation, position, &dx, &dy);

    if (ecs_entity_is_alive(simulation->world, food)) {
        if (abs(dx) > abs(dy)) {
            dx = sign_of(dx);
            dy = 0;
        } else if (dy != 0) {
            dx = 0;
            dy = sign_of(dy);
        } else {
            dx = sign_of(dx);
        }
    } else {
        unsigned direction = random_next(simulation) % 5u;
        static const int directions[5][2] = {
            {0, 0}, {1, 0}, {-1, 0}, {0, 1}, {0, -1}
        };
        dx = directions[direction][0];
        dy = directions[direction][1];
    }

    position->x = (position->x + dx + simulation->width) % simulation->width;
    position->y = (position->y + dy + simulation->height) % simulation->height;
}

static void eat_plant(Simulation *simulation, Position *position, Grazer *grazer)
{
    EcsEntity plant_entity = entity_at(
        simulation,
        position->x,
        position->y,
        simulation->plant
    );
    const Plant *plant;

    if (!ecs_entity_is_alive(simulation->world, plant_entity)) {
        return;
    }

    plant = ecs_component_get_const(simulation->world, plant_entity, simulation->plant);
    grazer->energy += plant->nutrition;
    ecs_entity_destroy(simulation->world, plant_entity);
}

static void reproduce_grazer(
    Simulation *simulation,
    Position *position,
    Grazer *grazer
)
{
    static const int neighbors[4][2] = {
        {1, 0}, {-1, 0}, {0, 1}, {0, -1}
    };
    unsigned start = random_next(simulation) % 4u;
    unsigned offset;

    if (grazer->energy < GRAZER_REPRODUCTION_ENERGY) {
        return;
    }

    for (offset = 0u; offset < 4u; ++offset) {
        unsigned neighbor = (start + offset) % 4u;
        int x = (position->x + neighbors[neighbor][0] + simulation->width) % simulation->width;
        int y = (position->y + neighbors[neighbor][1] + simulation->height) % simulation->height;
        int child_energy = grazer->energy / 2;

        if (ecs_entity_is_alive(
                simulation->world,
                spawn_grazer_at(simulation, x, y, child_energy)
            )) {
            grazer->energy -= child_energy;
            return;
        }
    }
}

static void grazer_system(Simulation *simulation)
{
    EcsIterator iterator = ecs_query(
        simulation->world,
        component_pair(simulation->position, simulation->grazer)
    );
    EcsEntity entity;
    size_t count = 0u;
    size_t index;

    while (count < ecs_world_capacity(simulation->world) &&
           ecs_query_next(&iterator, &entity)) {
        simulation->actor_snapshot[count] = entity;
        count++;
    }

    for (index = 0u; index < count; ++index) {
        Position *position;
        Grazer *grazer;

        entity = simulation->actor_snapshot[index];
        if (!ecs_entity_is_alive(simulation->world, entity)) {
            continue;
        }
        position = ecs_component_get(simulation->world, entity, simulation->position);
        grazer = ecs_component_get(simulation->world, entity, simulation->grazer);

        grazer->energy--;
        grazer->age++;
        if (grazer->energy <= 0 || grazer->age >= GRAZER_MAX_AGE) {
            ecs_entity_destroy(simulation->world, entity);
            continue;
        }

        move_grazer(simulation, position);
        eat_plant(simulation, position, grazer);
        reproduce_grazer(simulation, position, grazer);
    }
}

static void plant_growth_system(Simulation *simulation)
{
    size_t desired_growth = 1u + (size_t)(random_next(simulation) % 3u);
    (void)simulation_spawn(simulation, SIMULATION_PLANT, desired_growth);
}

Simulation *simulation_create(int width, int height, uint32_t seed)
{
    Simulation *simulation;
    size_t cell_count;

    if (width < 4 || height < 4 ||
        (size_t)width > SIZE_MAX / (size_t)height) {
        return NULL;
    }

    simulation = calloc(1u, sizeof(*simulation));
    if (simulation == NULL) {
        return NULL;
    }

    cell_count = (size_t)width * (size_t)height;
    simulation->world = ecs_world_create(SIMULATION_CAPACITY);
    simulation->actor_snapshot = malloc(
        SIMULATION_CAPACITY * sizeof(*simulation->actor_snapshot)
    );
    simulation->grid = malloc(cell_count);
    simulation->width = width;
    simulation->height = height;
    simulation->random_state = seed == 0u ? UINT32_C(0x9e3779b9) : seed;

    if (simulation->world == NULL || simulation->actor_snapshot == NULL ||
        simulation->grid == NULL) {
        simulation_destroy(simulation);
        return NULL;
    }

    simulation->position = ecs_component_register(simulation->world, sizeof(Position));
    simulation->appearance = ecs_component_register(simulation->world, sizeof(Appearance));
    simulation->plant = ecs_component_register(simulation->world, sizeof(Plant));
    simulation->grazer = ecs_component_register(simulation->world, sizeof(Grazer));
    if (simulation->position == ECS_COMPONENT_INVALID ||
        simulation->appearance == ECS_COMPONENT_INVALID ||
        simulation->plant == ECS_COMPONENT_INVALID ||
        simulation->grazer == ECS_COMPONENT_INVALID) {
        simulation_destroy(simulation);
        return NULL;
    }

    return simulation;
}

void simulation_destroy(Simulation *simulation)
{
    if (simulation == NULL) {
        return;
    }
    free(simulation->grid);
    free(simulation->actor_snapshot);
    ecs_world_destroy(simulation->world);
    free(simulation);
}

bool simulation_populate(Simulation *simulation, size_t plants, size_t grazers)
{
    size_t plant_count;
    size_t grazer_count;

    if (simulation == NULL) {
        return false;
    }
    plant_count = simulation_spawn(simulation, SIMULATION_PLANT, plants);
    grazer_count = simulation_spawn(simulation, SIMULATION_GRAZER, grazers);
    return plant_count == plants && grazer_count == grazers;
}

size_t simulation_spawn(
    Simulation *simulation,
    SimulationSpecies species,
    size_t count
)
{
    size_t spawned = 0u;

    if (simulation == NULL ||
        (species != SIMULATION_PLANT && species != SIMULATION_GRAZER)) {
        return 0u;
    }

    while (spawned < count) {
        int x;
        int y;
        EcsEntity entity;

        if (!find_open_cell(simulation, &x, &y)) {
            break;
        }
        if (species == SIMULATION_PLANT) {
            entity = spawn_plant_at(simulation, x, y);
        } else {
            entity = spawn_grazer_at(simulation, x, y, GRAZER_STARTING_ENERGY);
        }
        if (!ecs_entity_is_alive(simulation->world, entity)) {
            break;
        }
        spawned++;
    }
    return spawned;
}

void simulation_step(Simulation *simulation, size_t steps)
{
    size_t step;

    if (simulation == NULL) {
        return;
    }
    for (step = 0u; step < steps; ++step) {
        grazer_system(simulation);
        plant_growth_system(simulation);
        simulation->tick++;
    }
}

SimulationStats simulation_stats(Simulation *simulation)
{
    SimulationStats stats = {0u, 0u, 0u, 0u};
    EcsIterator iterator;
    EcsEntity entity;

    if (simulation == NULL) {
        return stats;
    }

    stats.tick = simulation->tick;
    stats.entities = ecs_entity_count(simulation->world);

    iterator = ecs_query(simulation->world, ecs_component_mask(simulation->plant));
    while (ecs_query_next(&iterator, &entity)) {
        stats.plants++;
    }
    iterator = ecs_query(simulation->world, ecs_component_mask(simulation->grazer));
    while (ecs_query_next(&iterator, &entity)) {
        stats.grazers++;
    }
    return stats;
}

void simulation_render(Simulation *simulation, FILE *output)
{
    EcsIterator iterator;
    EcsEntity entity;
    SimulationStats stats;
    int x;
    int y;

    if (simulation == NULL || output == NULL) {
        return;
    }

    memset(
        simulation->grid,
        ' ',
        (size_t)simulation->width * (size_t)simulation->height
    );
    iterator = ecs_query(
        simulation->world,
        component_pair(simulation->position, simulation->appearance)
    );
    while (ecs_query_next(&iterator, &entity)) {
        const Position *position = ecs_component_get_const(
            simulation->world,
            entity,
            simulation->position
        );
        const Appearance *appearance = ecs_component_get_const(
            simulation->world,
            entity,
            simulation->appearance
        );
        simulation->grid[
            (size_t)position->y * (size_t)simulation->width + (size_t)position->x
        ] = appearance->glyph;
    }

    stats = simulation_stats(simulation);
    fprintf(
        output,
        "\nTick %-5" PRIu64 "  plants: %-3zu  grazers: %-3zu\n",
        stats.tick,
        stats.plants,
        stats.grazers
    );
    fputc('+', output);
    for (x = 0; x < simulation->width; ++x) {
        fputc('-', output);
    }
    fputs("+\n", output);

    for (y = 0; y < simulation->height; ++y) {
        fputc('|', output);
        fwrite(
            simulation->grid + ((size_t)y * (size_t)simulation->width),
            1u,
            (size_t)simulation->width,
            output
        );
        fputs("|\n", output);
    }

    fputc('+', output);
    for (x = 0; x < simulation->width; ++x) {
        fputc('-', output);
    }
    fputs("+\n  * plant   g grazer\n", output);
}
