#include <flecs.h>
#include <stdio.h>

int main() {
  ecs_world_t *world = ecs_init();
  printf("Nihao...\n");

  ECS_COMPONENT(Brain);

  ecs_entity_t brian = ecs_new(world);
  ecs_add(world, brian, Brain);

  ecs_fini(world);
  return 0;
}
