#include "ecs.h"

#include <assert.h>
#include <stdio.h>

typedef struct {
    int x;
    int y;
} Position;

typedef struct {
    int value;
} Health;

static void test_entity_lifecycle(void)
{
    EcsWorld *world = ecs_world_create(2u);
    EcsComponent position_id;
    EcsEntity first;
    EcsEntity replacement;
    Position *position;

    assert(world != NULL);
    position_id = ecs_component_register(world, sizeof(Position));
    assert(position_id != ECS_COMPONENT_INVALID);

    first = ecs_entity_create(world);
    assert(ecs_entity_is_alive(world, first));
    assert(ecs_entity_count(world) == 1u);

    position = ecs_component_add(world, first, position_id);
    assert(position != NULL);
    assert(position->x == 0 && position->y == 0);
    position->x = 4;
    position->y = 7;
    assert(((Position *)ecs_component_get(world, first, position_id))->x == 4);

    assert(ecs_entity_destroy(world, first));
    assert(!ecs_entity_is_alive(world, first));
    assert(ecs_component_get(world, first, position_id) == NULL);
    assert(ecs_component_add(world, first, position_id) == NULL);

    replacement = ecs_entity_create(world);
    assert(replacement.index == first.index);
    assert(replacement.generation != first.generation);
    assert(ecs_entity_is_alive(world, replacement));
    assert(!ecs_entity_destroy(world, first));

    ecs_world_destroy(world);
}

static void test_components_and_queries(void)
{
    EcsWorld *world = ecs_world_create(8u);
    EcsComponent position_id;
    EcsComponent health_id;
    EcsEntity moving;
    EcsEntity stationary;
    EcsIterator iterator;
    EcsEntity found;
    size_t count = 0u;

    assert(world != NULL);
    position_id = ecs_component_register(world, sizeof(Position));
    health_id = ecs_component_register(world, sizeof(Health));
    assert(position_id != ECS_COMPONENT_INVALID);
    assert(health_id != ECS_COMPONENT_INVALID);

    moving = ecs_entity_create(world);
    stationary = ecs_entity_create(world);
    assert(ecs_component_add(world, moving, position_id) != NULL);
    assert(ecs_component_add(world, moving, health_id) != NULL);
    assert(ecs_component_add(world, stationary, position_id) != NULL);

    iterator = ecs_query(
        world,
        ecs_component_mask(position_id) | ecs_component_mask(health_id)
    );
    while (ecs_query_next(&iterator, &found)) {
        assert(found.index == moving.index);
        count++;
    }
    assert(count == 1u);

    assert(ecs_component_remove(world, moving, health_id));
    assert(!ecs_component_remove(world, moving, health_id));
    assert(ecs_component_get(world, moving, health_id) == NULL);

    iterator = ecs_query(world, ecs_component_mask(position_id));
    count = 0u;
    while (ecs_query_next(&iterator, &found)) {
        count++;
    }
    assert(count == 2u);

    ecs_world_destroy(world);
}

static void test_capacity(void)
{
    EcsWorld *world = ecs_world_create(1u);
    EcsEntity first;
    EcsEntity overflow;

    assert(world != NULL);
    first = ecs_entity_create(world);
    overflow = ecs_entity_create(world);
    assert(ecs_entity_is_alive(world, first));
    assert(!ecs_entity_is_alive(world, overflow));
    assert(ecs_world_capacity(world) == 1u);
    ecs_world_destroy(world);
}

int main(void)
{
    test_entity_lifecycle();
    test_components_and_queries();
    test_capacity();
    puts("ecs tests passed");
    return 0;
}

