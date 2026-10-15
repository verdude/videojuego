#pragma once

#include "../graphics/texturestore.hpp"
#include "spritesheet.hpp"

#include <SDL3/SDL.h>
#include <unordered_map>

class AssetStore
{
private:
  TextureStore texture_store;
  std::unordered_map<const char*, SpriteSheet> sprite_sheets;

public:
  AssetStore();
  bool init();
};
