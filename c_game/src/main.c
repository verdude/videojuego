#include <flecs.h>
#include <stdio.h>

typedef struct {
  int range;
  long bps;
  int version;
} BLE, WFD, UWB;

int main() {
  ecs_world_t *world = ecs_init();

  ECS_COMPONENT(world, BLE);
  ECS_COMPONENT(world, WFD);
  ECS_COMPONENT(world, UWB);

  ecs_entity_t brian = ecs_new(world);
  ecs_add(world, brian, BLE);

  ecs_fini(world);
  printf("Nihao...\n");
  return 0;
}
