#pragma once

#include "texturestore.hpp"

#include <SDL3/SDL.h>

// Wrapper classes for automatic ordered destruction
class Renderer
{
public:
  SDL_Renderer* r;
  Renderer(SDL_Renderer* renderer)
    : r(renderer)
  {
  }
  ~Renderer()
  {
    if (r) {
      SDL_DestroyRenderer(r);
    }
  }
};

class Window
{
public:
  SDL_Window* w;

  Window(SDL_Window* window)
    : w(window)
  {
  }
  ~Window()
  {
    if (w) {
      SDL_DestroyWindow(w);
    }
  }
};

class Graphics
{
private:
  Window window;
  Renderer renderer;
  TextureStore texture_store;

public:
  Graphics();
  bool init();
  void debug(const char*, bool = true);
  void present();
};
