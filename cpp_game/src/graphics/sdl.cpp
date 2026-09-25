#include "sdl.hpp"

#include <iostream>

SDL::SDL() : window(nullptr), renderer(nullptr) {}

SDL::~SDL() {
  if (renderer) {
    SDL_DestroyRenderer(renderer);
  }
  if (window) {
    SDL_DestroyWindow(window);
  }
  SDL_Quit();
}

bool SDL::init() {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    std::cerr << "Failed to initialize SDL: " << SDL_GetError() << "\n";
    return false;
  }

  if (!SDL_CreateWindowAndRenderer("Game", 800, 800, 0, &window, &renderer)) {
    std::cerr << "Failed to create window and renderer: " << SDL_GetError() << "\n";
    return false;
  }

  if (!SDL_SetRenderVSync(renderer, 1)) {
    std::cerr << "vsync unsupported" << SDL_GetError() << "\n";
  }

  return true;
}

void SDL::debug(char* message, bool clear) {
  if (clear) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);
  }
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
    SDL_RenderDebugText(renderer, 100, 100, message);
}

void SDL::present() {
    SDL_RenderPresent(renderer);
}
