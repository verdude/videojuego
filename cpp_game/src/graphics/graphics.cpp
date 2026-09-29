#include "graphics.hpp"

#include <iostream>

Graphics::Graphics()
  : window(nullptr)
  , renderer(nullptr)
  , texture_store()
{
}

bool
Graphics::init()
{
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    std::cerr << "Failed to initialize Graphics: " << SDL_GetError() << "\n";
    return false;
  }

  if (!SDL_CreateWindowAndRenderer(
        "Game", 800, 800, 0, &window.w, &renderer.r)) {
    std::cerr << "Failed to create window and renderer: " << SDL_GetError()
              << "\n";
    return false;
  }

  if (!SDL_SetRenderVSync(renderer.r, 1)) {
    std::cerr << "vsync unsupported" << SDL_GetError() << "\n";
  }

  texture_store.init(renderer.r);

  return true;
}

void
Graphics::debug(const char* message, bool clear)
{
  if (clear) {
    SDL_SetRenderDrawColor(renderer.r, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer.r);
  }
  SDL_SetRenderDrawColor(renderer.r, 255, 255, 255, SDL_ALPHA_OPAQUE);
  SDL_RenderDebugText(renderer.r, 100, 100, message);
}

void
Graphics::present()
{
  SDL_RenderPresent(renderer.r);
}
