#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <chrono>
#include <iostream>
#include <vector>

#include "combat/scene.hpp"
#include "graphics/sdl.hpp"
#include "input/inputcontroller.hpp"

int
main()
{
  SDL graphics;
  if (!graphics.init()) {
    return 1;
  }

  std::vector<SDL_Event> events;
  using Clock = std::chrono::steady_clock;
  auto current_time = Clock::now();

  // 60 ticks per second
  double tick_duration = 1.0 / 60;
  auto simulatable_time = 0.0;

  FightScene scene = localFight();

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
      scene.update();
      simulatable_time -= tick_duration;
    }

    // render
    graphics.debug("welcome");
    graphics.present();
  }
}
