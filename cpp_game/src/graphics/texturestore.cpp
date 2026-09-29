#include "texturestore.hpp"

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <string.h>
#include <string>
#include <unordered_map>

bool
TextureStore::load_from_file(const char* path, SDL_Renderer*& renderer)
{
  SDL_Texture* t = IMG_LoadTexture(renderer, path);
  if (!t) {
    return false;
  }
  textures.insert({ textures.size(), Texture{ path, t } });
  return true;
}

void
TextureStore::cleanup()
{
  for (auto& p : textures) {
    if (p.second.texture != NULL) {
      SDL_DestroyTexture(p.second.texture);
    }
  }
  textures.clear();
}

TextureStore::TextureStore()
  : textures()
{
}

TextureStore::~TextureStore()
{
  cleanup();
}

bool
TextureStore::init(SDL_Renderer*& renderer)
{
  // TODO: windows?
  std::string path = "assets/";
  for (const auto& entry : std::filesystem::directory_iterator(path)) {
    const auto& p = entry.path();
    if (entry.is_regular_file() && p.extension() == ".png") {
      std::string name = p.string();
      std::cout << "Loading png as texture: " << name << "\n";
      if (!load_from_file(name.c_str(), renderer)) {
        cleanup();
        return false;
      }
    }
  }
  return true;
}
