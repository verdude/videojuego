#include "simulation.h"

#include <flecs.h>

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
    ecs_world_t *world;
    ecs_entity_t position;
    ecs_entity_t appearance;
    ecs_entity_t plant;
    ecs_entity_t grazer;
    ecs_query_t *positions;
    ecs_query_t *plants;
    ecs_query_t *grazers;
    ecs_query_t *renderables;
    ecs_entity_t *actor_snapshot;
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

static bool entity_is_alive(const Simulation *simulation, ecs_entity_t entity)
{
    return entity != 0u && ecs_is_alive(simulation->world, entity);
}

static size_t query_count(Simulation *simulation, const ecs_query_t *query)
{
    ecs_iter_t iterator = ecs_query_iter(simulation->world, query);
    size_t count = 0u;

    while (ecs_query_next(&iterator)) {
        count += (size_t)iterator.count;
    }
    return count;
}

static ecs_entity_t plant_at(Simulation *simulation, int x, int y)
{
    ecs_iter_t iterator = ecs_query_iter(simulation->world, simulation->plants);

    while (ecs_query_next(&iterator)) {
        const Position *positions = ecs_field(&iterator, Position, 0);
        int32_t index;

        for (index = 0; index < iterator.count; ++index) {
            if (positions[index].x == x && positions[index].y == y) {
                ecs_entity_t entity = iterator.entities[index];
                ecs_iter_fini(&iterator);
                return entity;
            }
        }
    }
    return 0u;
}

static bool cell_is_open(Simulation *simulation, int x, int y)
{
    ecs_iter_t iterator = ecs_query_iter(simulation->world, simulation->positions);

    while (ecs_query_next(&iterator)) {
        const Position *positions = ecs_field(&iterator, Position, 0);
        int32_t index;

        for (index = 0; index < iterator.count; ++index) {
            if (positions[index].x == x && positions[index].y == y) {
                ecs_iter_fini(&iterator);
                return false;
            }
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

static ecs_entity_t spawn_plant_at(Simulation *simulation, int x, int y)
{
    ecs_entity_t entity;
    Position position = {x, y};
    Appearance appearance = {'*'};
    Plant plant = {PLANT_NUTRITION};

    if (query_count(simulation, simulation->positions) >= SIMULATION_CAPACITY ||
        !cell_is_open(simulation, x, y)) {
        return 0u;
    }

    entity = ecs_new(simulation->world);
    ecs_set_id(
        simulation->world,
        entity,
        simulation->position,
        sizeof(position),
        &position
    );
    ecs_set_id(
        simulation->world,
        entity,
        simulation->appearance,
        sizeof(appearance),
        &appearance
    );
    ecs_set_id(
        simulation->world,
        entity,
        simulation->plant,
        sizeof(plant),
        &plant
    );
    return entity;
}

static ecs_entity_t spawn_grazer_at(
    Simulation *simulation,
    int x,
    int y,
    int starting_energy
)
{
    ecs_entity_t entity;
    Position position = {x, y};
    Appearance appearance = {'g'};
    Grazer grazer = {starting_energy, 0};

    if (query_count(simulation, simulation->positions) >= SIMULATION_CAPACITY ||
        !cell_is_open(simulation, x, y)) {
        return 0u;
    }

    entity = ecs_new(simulation->world);
    ecs_set_id(
        simulation->world,
        entity,
        simulation->position,
        sizeof(position),
        &position
    );
    ecs_set_id(
        simulation->world,
        entity,
        simulation->appearance,
        sizeof(appearance),
        &appearance
    );
    ecs_set_id(
        simulation->world,
        entity,
        simulation->grazer,
        sizeof(grazer),
        &grazer
    );
    return entity;
}

static ecs_entity_t nearest_plant(
    Simulation *simulation,
    const Position *origin,
    int *distance_x,
    int *distance_y
)
{
    ecs_iter_t iterator = ecs_query_iter(simulation->world, simulation->plants);
    ecs_entity_t nearest = 0u;
    int nearest_distance = INT_MAX;

    while (ecs_query_next(&iterator)) {
        const Position *positions = ecs_field(&iterator, Position, 0);
        int32_t index;

        for (index = 0; index < iterator.count; ++index) {
            int dx = positions[index].x - origin->x;
            int dy = positions[index].y - origin->y;
            int distance = abs(dx) + abs(dy);

            if (distance < nearest_distance) {
                nearest = iterator.entities[index];
                nearest_distance = distance;
                *distance_x = dx;
                *distance_y = dy;
            }
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
    ecs_entity_t food = nearest_plant(simulation, position, &dx, &dy);

    if (entity_is_alive(simulation, food)) {
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

static int eat_plant(Simulation *simulation, const Position *position)
{
    ecs_entity_t plant_entity = plant_at(simulation, position->x, position->y);
    const Plant *plant;
    int nutrition;

    if (!entity_is_alive(simulation, plant_entity)) {
        return 0;
    }

    plant = ecs_get_id(simulation->world, plant_entity, simulation->plant);
    nutrition = plant->nutrition;
    ecs_delete(simulation->world, plant_entity);
    return nutrition;
}

static void reproduce_grazer(Simulation *simulation, ecs_entity_t entity)
{
    static const int neighbors[4][2] = {
        {1, 0}, {-1, 0}, {0, 1}, {0, -1}
    };
    const Position *position_component = ecs_get_id(
        simulation->world,
        entity,
        simulation->position
    );
    const Grazer *grazer_component = ecs_get_id(
        simulation->world,
        entity,
        simulation->grazer
    );
    Position position;
    int energy;
    unsigned start;
    unsigned offset;

    if (position_component == NULL || grazer_component == NULL) {
        return;
    }
    position = *position_component;
    energy = grazer_component->energy;
    if (energy < GRAZER_REPRODUCTION_ENERGY) {
        return;
    }

    start = random_next(simulation) % 4u;
    for (offset = 0u; offset < 4u; ++offset) {
        unsigned neighbor = (start + offset) % 4u;
        int x = (position.x + neighbors[neighbor][0] + simulation->width) %
            simulation->width;
        int y = (position.y + neighbors[neighbor][1] + simulation->height) %
            simulation->height;
        int child_energy = energy / 2;
        ecs_entity_t child = spawn_grazer_at(simulation, x, y, child_energy);

        if (entity_is_alive(simulation, child)) {
            Grazer *parent = ecs_get_mut_id(
                simulation->world,
                entity,
                simulation->grazer
            );
            parent->energy -= child_energy;
            ecs_modified_id(simulation->world, entity, simulation->grazer);
            return;
        }
    }
}

static void grazer_system(Simulation *simulation)
{
    ecs_iter_t iterator = ecs_query_iter(simulation->world, simulation->grazers);
    size_t count = 0u;
    size_t snapshot_index;

    while (ecs_query_next(&iterator)) {
        int32_t index;

        for (index = 0; index < iterator.count; ++index) {
            if (count < SIMULATION_CAPACITY) {
                simulation->actor_snapshot[count] = iterator.entities[index];
                count++;
            }
        }
    }

    for (snapshot_index = 0u; snapshot_index < count; ++snapshot_index) {
        ecs_entity_t entity = simulation->actor_snapshot[snapshot_index];
        Position *position;
        Grazer *grazer;
        int nutrition;

        if (!entity_is_alive(simulation, entity)) {
            continue;
        }

        grazer = ecs_get_mut_id(simulation->world, entity, simulation->grazer);
        grazer->energy--;
        grazer->age++;
        if (grazer->energy <= 0 || grazer->age >= GRAZER_MAX_AGE) {
            ecs_delete(simulation->world, entity);
            continue;
        }

        position = ecs_get_mut_id(simulation->world, entity, simulation->position);
        move_grazer(simulation, position);
        ecs_modified_id(simulation->world, entity, simulation->position);

        nutrition = eat_plant(simulation, position);
        grazer = ecs_get_mut_id(simulation->world, entity, simulation->grazer);
        grazer->energy += nutrition;
        ecs_modified_id(simulation->world, entity, simulation->grazer);
        reproduce_grazer(simulation, entity);
    }
}

static void plant_growth_system(Simulation *simulation)
{
    size_t desired_growth = 1u + (size_t)(random_next(simulation) % 3u);
    (void)simulation_spawn(simulation, SIMULATION_PLANT, desired_growth);
}

static ecs_entity_t register_component(
    ecs_world_t *world,
    const char *name,
    ecs_size_t size,
    ecs_size_t alignment
)
{
    ecs_entity_t entity = ecs_entity(world, {.name = name});
    return ecs_component(world, {
        .entity = entity,
        .type = {.size = size, .alignment = alignment}
    });
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
    simulation->world = ecs_init();
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

    simulation->position = register_component(
        simulation->world,
        "Position",
        (ecs_size_t)sizeof(Position),
        (ecs_size_t)_Alignof(Position)
    );
    simulation->appearance = register_component(
        simulation->world,
        "Appearance",
        (ecs_size_t)sizeof(Appearance),
        (ecs_size_t)_Alignof(Appearance)
    );
    simulation->plant = register_component(
        simulation->world,
        "Plant",
        (ecs_size_t)sizeof(Plant),
        (ecs_size_t)_Alignof(Plant)
    );
    simulation->grazer = register_component(
        simulation->world,
        "Grazer",
        (ecs_size_t)sizeof(Grazer),
        (ecs_size_t)_Alignof(Grazer)
    );

    simulation->positions = ecs_query(simulation->world, {
        .terms = {{.id = simulation->position}}
    });
    simulation->plants = ecs_query(simulation->world, {
        .terms = {
            {.id = simulation->position},
            {.id = simulation->plant}
        }
    });
    simulation->grazers = ecs_query(simulation->world, {
        .terms = {
            {.id = simulation->position},
            {.id = simulation->grazer}
        }
    });
    simulation->renderables = ecs_query(simulation->world, {
        .terms = {
            {.id = simulation->position},
            {.id = simulation->appearance}
        }
    });

    if (simulation->position == 0u || simulation->appearance == 0u ||
        simulation->plant == 0u || simulation->grazer == 0u ||
        simulation->positions == NULL || simulation->plants == NULL ||
        simulation->grazers == NULL || simulation->renderables == NULL) {
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

    if (simulation->positions != NULL) {
        ecs_query_fini(simulation->positions);
    }
    if (simulation->plants != NULL) {
        ecs_query_fini(simulation->plants);
    }
    if (simulation->grazers != NULL) {
        ecs_query_fini(simulation->grazers);
    }
    if (simulation->renderables != NULL) {
        ecs_query_fini(simulation->renderables);
    }
    if (simulation->world != NULL) {
        (void)ecs_fini(simulation->world);
    }
    free(simulation->grid);
    free(simulation->actor_snapshot);
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
        ecs_entity_t entity;

        if (!find_open_cell(simulation, &x, &y)) {
            break;
        }
        if (species == SIMULATION_PLANT) {
            entity = spawn_plant_at(simulation, x, y);
        } else {
            entity = spawn_grazer_at(simulation, x, y, GRAZER_STARTING_ENERGY);
        }
        if (!entity_is_alive(simulation, entity)) {
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

    if (simulation == NULL) {
        return stats;
    }

    stats.tick = simulation->tick;
    stats.plants = query_count(simulation, simulation->plants);
    stats.grazers = query_count(simulation, simulation->grazers);
    stats.entities = query_count(simulation, simulation->positions);
    return stats;
}

void simulation_render(Simulation *simulation, FILE *output)
{
    ecs_iter_t iterator;
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
    iterator = ecs_query_iter(simulation->world, simulation->renderables);
    while (ecs_query_next(&iterator)) {
        const Position *positions = ecs_field(&iterator, Position, 0);
        const Appearance *appearances = ecs_field(&iterator, Appearance, 1);
        int32_t index;

        for (index = 0; index < iterator.count; ++index) {
            simulation->grid[
                (size_t)positions[index].y * (size_t)simulation->width +
                (size_t)positions[index].x
            ] = appearances[index].glyph;
        }
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
