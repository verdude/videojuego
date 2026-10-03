#pragma once

#include <SDL3/SDL.h>
#include <unordered_map>
#include "texturestore.hpp"

class AssetStore {
  private:
    TextureStore texture_store;
    std::unordered_map<const char*, SpriteSheet> sprite_sheets;

  public:
    AssetStore();
    bool init(SDL_Renderer*&);
}
