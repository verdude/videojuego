#pragma once

#include "../geometry/types.hpp"
#include "texturestore.hpp"

#include <SDL3/SDL.h>

struct AnimationFrame;

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

  bool initTextures();

public:
  Graphics();
  bool init();
  SDL_Renderer* get_renderer() const;
  void debug(const char*, bool = true);
  void present();
  void clear();
  void render_texture(const AnimationFrame*, const Rect*);
};
