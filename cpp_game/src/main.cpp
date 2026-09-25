#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <chrono>
#include <iostream>

#include "graphics/sdl.hpp"
#include "input/Input.hpp"

int
main(int argc, char** argv)
{
  SDL sdl;
  if (!sdl.init()) {
    return 1;
  }

  using Clock = std::chrono::steady_clock;
  auto current_time = Clock::now();

  // 60 ticks per second
  double tick_duration = 1.0 / 60;
  auto simulatable_time = 0.0;

  bool play = true;

  while (play) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT) {
        play = false;
      }
    }

    auto new_time = Clock::now();
    double elapsed_time =
      std::chrono::duration<double>(new_time - current_time).count();
    current_time = new_time;
    simulatable_time += elapsed_time;

    while (simulatable_time >= tick_duration) {
      // simulate gameplay
      std::cout << elapsed_time << "\n";
      simulatable_time -= tick_duration;
    }

    // render
    sdl.debug("welcome");
    sdl.present();
  }

  return 0;
}
