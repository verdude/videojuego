#include <iostream>
#include <chrono>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "input/Input.hpp"

static SDL_Window* window = nullptr;
static SDL_Renderer* renderer = nullptr;

static bool initialize_sdl() {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    std::cerr << "Failed to initialize SDL: " << SDL_GetError() << "\n";
    SDL_Quit();
    return false;
  }

  if (!SDL_CreateWindowAndRenderer("Game", 800, 800, 0, &window, &renderer)) {
    std::cerr << "Failed to create window and renderer: " << SDL_GetError() << "\n";
    SDL_Quit();
    return false;
  }

  if (!SDL_SetRenderVSync(renderer, 1)) {
    std::cerr << "vsync unsupported" << SDL_GetError() << "\n";
  }

  return true;
}

int main(int argc, char** argv) {
  if (!initialize_sdl()) {
    return 1;
  }

  using Clock = std::chrono::steady_clock;
  auto current_time = Clock::now();

  // 60 ticks per second
  double tick_duration = 1.0/60;
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
    double elapsed_time = std::chrono::duration<double>(new_time-current_time).count();
    current_time = new_time;
    simulatable_time += elapsed_time;

    while (simulatable_time >= tick_duration) {
      // simulate gameplay
      std::cout << elapsed_time << "\n";
      simulatable_time -= tick_duration;
    }

    // render
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
    SDL_RenderDebugText(renderer, 100, 100, "Welcome");
    SDL_RenderPresent(renderer);
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
