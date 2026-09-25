#pragma once

#include <SDL3/SDL.h>

class SDL {
  SDL_Window* window;
  SDL_Renderer* renderer;

public:
  SDL();
  ~SDL();
  bool init();
  void debug(char*, bool);
  void present();
};
