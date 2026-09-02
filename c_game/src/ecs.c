#include "ecs.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    uint32_t generation;
    uint64_t components;
    bool alive;
} EcsRecord;

typedef struct {
    unsigned char *data;
    size_t element_size;
} EcsComponentPool;

struct EcsWorld {
    size_t capacity;
    size_t count;
    size_t next_unused;
    size_t free_count;
    EcsRecord *records;
    uint32_t *free_indices;
    EcsComponentPool pools[ECS_MAX_COMPONENTS];
    EcsComponent component_count;
};

static bool component_is_registered(const EcsWorld *world, EcsComponent component)
{
    return world != NULL && component < world->component_count;
}

static void *component_address(
    const EcsWorld *world,
    EcsEntity entity,
    EcsComponent component
)
{
    const EcsComponentPool *pool = &world->pools[component];
    return pool->data + ((size_t)entity.index * pool->element_size);
}

EcsWorld *ecs_world_create(size_t capacity)
{
    EcsWorld *world;

    if (capacity == 0u || capacity > UINT32_MAX ||
        capacity > SIZE_MAX / sizeof(*world->free_indices)) {
        return NULL;
    }

    world = calloc(1u, sizeof(*world));
    if (world == NULL) {
        return NULL;
    }

    world->records = calloc(capacity, sizeof(*world->records));
    world->free_indices = malloc(capacity * sizeof(*world->free_indices));
    if (world->records == NULL || world->free_indices == NULL) {
        ecs_world_destroy(world);
        return NULL;
    }

    world->capacity = capacity;
    return world;
}

void ecs_world_destroy(EcsWorld *world)
{
    size_t component;

    if (world == NULL) {
        return;
    }

    for (component = 0u; component < world->component_count; ++component) {
        free(world->pools[component].data);
    }
    free(world->free_indices);
    free(world->records);
    free(world);
}

size_t ecs_world_capacity(const EcsWorld *world)
{
    return world == NULL ? 0u : world->capacity;
}

size_t ecs_entity_count(const EcsWorld *world)
{
    return world == NULL ? 0u : world->count;
}

EcsComponent ecs_component_register(EcsWorld *world, size_t component_size)
{
    EcsComponent component;
    unsigned char *data;

    if (world == NULL || component_size == 0u ||
        world->component_count >= ECS_MAX_COMPONENTS ||
        component_size > SIZE_MAX / world->capacity) {
        return ECS_COMPONENT_INVALID;
    }

    data = calloc(world->capacity, component_size);
    if (data == NULL) {
        return ECS_COMPONENT_INVALID;
    }

    component = world->component_count;
    world->component_count++;
    world->pools[component].data = data;
    world->pools[component].element_size = component_size;
    return component;
}

uint64_t ecs_component_mask(EcsComponent component)
{
    if (component >= ECS_MAX_COMPONENTS) {
        return UINT64_C(0);
    }
    return UINT64_C(1) << component;
}

EcsEntity ecs_entity_create(EcsWorld *world)
{
    EcsEntity entity = ECS_ENTITY_INVALID;
    EcsRecord *record;

    if (world == NULL || world->count == world->capacity) {
        return entity;
    }

    if (world->free_count > 0u) {
        world->free_count--;
        entity.index = world->free_indices[world->free_count];
    } else {
        entity.index = (uint32_t)world->next_unused;
        world->next_unused++;
    }

    record = &world->records[entity.index];
    if (record->generation == 0u) {
        record->generation = 1u;
    }
    record->alive = true;
    record->components = UINT64_C(0);
    world->count++;

    entity.generation = record->generation;
    return entity;
}

bool ecs_entity_destroy(EcsWorld *world, EcsEntity entity)
{
    EcsRecord *record;
    size_t component;

    if (!ecs_entity_is_alive(world, entity)) {
        return false;
    }

    record = &world->records[entity.index];
    for (component = 0u; component < world->component_count; ++component) {
        uint64_t mask = UINT64_C(1) << component;
        if ((record->components & mask) != 0u) {
            EcsComponent id = (EcsComponent)component;
            memset(component_address(world, entity, id), 0, world->pools[id].element_size);
        }
    }

    record->alive = false;
    record->components = UINT64_C(0);
    record->generation++;
    if (record->generation == 0u) {
        record->generation = 1u;
    }

    world->free_indices[world->free_count] = entity.index;
    world->free_count++;
    world->count--;
    return true;
}

bool ecs_entity_is_alive(const EcsWorld *world, EcsEntity entity)
{
    const EcsRecord *record;

    if (world == NULL || entity.index >= world->capacity) {
        return false;
    }
    record = &world->records[entity.index];
    return record->alive && record->generation == entity.generation;
}

void *ecs_component_add(EcsWorld *world, EcsEntity entity, EcsComponent component)
{
    EcsRecord *record;
    void *address;
    uint64_t mask;

    if (!ecs_entity_is_alive(world, entity) ||
        !component_is_registered(world, component)) {
        return NULL;
    }

    record = &world->records[entity.index];
    mask = ecs_component_mask(component);
    address = component_address(world, entity, component);
    if ((record->components & mask) == 0u) {
        memset(address, 0, world->pools[component].element_size);
        record->components |= mask;
    }
    return address;
}

bool ecs_component_remove(EcsWorld *world, EcsEntity entity, EcsComponent component)
{
    EcsRecord *record;
    uint64_t mask;

    if (!ecs_entity_is_alive(world, entity) ||
        !component_is_registered(world, component)) {
        return false;
    }

    record = &world->records[entity.index];
    mask = ecs_component_mask(component);
    if ((record->components & mask) == 0u) {
        return false;
    }

    memset(
        component_address(world, entity, component),
        0,
        world->pools[component].element_size
    );
    record->components &= ~mask;
    return true;
}

void *ecs_component_get(EcsWorld *world, EcsEntity entity, EcsComponent component)
{
    if (!ecs_entity_is_alive(world, entity) ||
        !component_is_registered(world, component) ||
        (world->records[entity.index].components & ecs_component_mask(component)) == 0u) {
        return NULL;
    }
    return component_address(world, entity, component);
}

const void *ecs_component_get_const(
    const EcsWorld *world,
    EcsEntity entity,
    EcsComponent component
)
{
    if (!ecs_entity_is_alive(world, entity) ||
        !component_is_registered(world, component) ||
        (world->records[entity.index].components & ecs_component_mask(component)) == 0u) {
        return NULL;
    }
    return component_address(world, entity, component);
}

EcsIterator ecs_query(EcsWorld *world, uint64_t required_components)
{
    EcsIterator iterator;
    iterator.world = world;
    iterator.required = required_components;
    iterator.cursor = 0u;
    return iterator;
}

bool ecs_query_next(EcsIterator *iterator, EcsEntity *entity)
{
    EcsWorld *world;

    if (iterator == NULL || entity == NULL || iterator->world == NULL) {
        return false;
    }
    world = iterator->world;

    while (iterator->cursor < world->next_unused) {
        size_t index = iterator->cursor;
        EcsRecord *record = &world->records[index];
        iterator->cursor++;

        if (record->alive &&
            (record->components & iterator->required) == iterator->required) {
            entity->index = (uint32_t)index;
            entity->generation = record->generation;
            return true;
        }
    }
    return false;
}
