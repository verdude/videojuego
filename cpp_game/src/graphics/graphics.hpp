#pragma once

#include <SDL3/SDL.h>

class Graphics
{
private:
  SDL_Window* window;
  SDL_Renderer* renderer;

public:
  Graphics();
  ~Graphics();
  bool init();
  void debug(const char*, bool = true);
  void present();
};
