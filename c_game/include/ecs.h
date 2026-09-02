#ifndef C_GAME_ECS_H
#define C_GAME_ECS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define ECS_MAX_COMPONENTS 64u
#define ECS_COMPONENT_INVALID UINT8_MAX

typedef uint8_t EcsComponent;

typedef struct {
    uint32_t index;
    uint32_t generation;
} EcsEntity;

#define ECS_ENTITY_INVALID ((EcsEntity){UINT32_MAX, 0u})

typedef struct EcsWorld EcsWorld;

typedef struct {
    EcsWorld *world;
    uint64_t required;
    size_t cursor;
} EcsIterator;

EcsWorld *ecs_world_create(size_t capacity);
void ecs_world_destroy(EcsWorld *world);

size_t ecs_world_capacity(const EcsWorld *world);
size_t ecs_entity_count(const EcsWorld *world);

EcsComponent ecs_component_register(EcsWorld *world, size_t component_size);
uint64_t ecs_component_mask(EcsComponent component);

EcsEntity ecs_entity_create(EcsWorld *world);
bool ecs_entity_destroy(EcsWorld *world, EcsEntity entity);
bool ecs_entity_is_alive(const EcsWorld *world, EcsEntity entity);

void *ecs_component_add(EcsWorld *world, EcsEntity entity, EcsComponent component);
bool ecs_component_remove(EcsWorld *world, EcsEntity entity, EcsComponent component);
void *ecs_component_get(EcsWorld *world, EcsEntity entity, EcsComponent component);
const void *ecs_component_get_const(
    const EcsWorld *world,
    EcsEntity entity,
    EcsComponent component
);

EcsIterator ecs_query(EcsWorld *world, uint64_t required_components);
bool ecs_query_next(EcsIterator *iterator, EcsEntity *entity);

#endif

