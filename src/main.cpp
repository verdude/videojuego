#include "animation/assetstore.hpp"
#include "combat/fighter.hpp"
#include "combat/scene.hpp"
#include "graphics/graphics.hpp"
#include "input/inputcontroller.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <chrono>
#include <iostream>
#include <memory>
#include <vector>

static std::shared_ptr<FighterDefinition>
init_moves(SpriteSheet* sheet)
{
  MoveDefinition punch =
    MoveDefinition(2, 1, 1, 1, sheet->get_animation("punch"));
  MoveDefinition walk =
    MoveDefinition(2, 1, 1, 1, sheet->get_animation("walk"));
  MoveDefinition neutral =
    MoveDefinition(2, 1, 1, 1, sheet->get_animation("neutral"));
  MoveDefinition block =
    MoveDefinition(2, 1, 1, 1, sheet->get_animation("block"));
  return std::make_shared<FighterDefinition>(punch, walk, neutral, block);
}

static std::shared_ptr<FighterDefinition>
load_fighter_definition(AssetStore& asset_store, const char* name = "pikachu")
{
  auto sheet = asset_store.get_sheet(name);
  return init_moves(sheet);
}

static bool
load_assets(AssetStore& asset_store, SDL_Renderer* renderer)
{
  return asset_store.init(renderer);
}

int
main()
{
  Graphics graphics;
  if (!graphics.init()) {
    return 1;
  }

  AssetStore asset_store = AssetStore();
  if (!load_assets(asset_store, graphics.get_renderer())) {
    return 1;
  }

  std::vector<SDL_Event> events;
  using Clock = std::chrono::steady_clock;
  auto current_time = Clock::now();

  // 60 ticks per second
  double tick_duration = 1.0 / 60;
  auto simulatable_time = 0.0;

  std::shared_ptr<FighterDefinition> pikachu =
    load_fighter_definition(asset_store);
  FightScene scene = localFight(pikachu, pikachu);

  while (true) {
    auto new_time = Clock::now();
    double elapsed_time =
      std::chrono::duration<double>(new_time - current_time).count();
    current_time = new_time;
    simulatable_time += elapsed_time;

    SDL_Event event;
    // TODO: when to clear events
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT) {
        return 0;
      } else {
        // collect events
        events.push_back(event);
      }
    }

    while (simulatable_time >= tick_duration) {
      scene.ingest(events);
      events.clear();
      scene.update();
      simulatable_time -= tick_duration;
    }

    // render
    graphics.clear();
    scene.render(graphics, asset_store);
    graphics.debug("welcome", false);
    graphics.present();
  }
}
