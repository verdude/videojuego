#pragma once

#include "../graphics/texturestore.hpp"
#include "spritesheet.hpp"

#include <SDL3/SDL.h>
#include <string>
#include <unordered_map>

class AssetStore
{
private:
  TextureStore texture_store;
  std::unordered_map<std::string, SpriteSheet> sprite_sheets;

public:
  AssetStore();
  bool init(SDL_Renderer*);
  SpriteSheet* get_sheet(const char*);
};
