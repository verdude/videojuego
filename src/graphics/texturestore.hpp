#pragma once

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <string>
#include <unordered_map>

struct Texture
{
  const std::string name;
  SDL_Texture* texture;
};

struct LoadTextureResult {
  bool success;
  size_t key;
};

class TextureStore
{
private:
  std::unordered_map<int, Texture> textures;

  LoadTextureResult load_from_file(const char*, SDL_Renderer*&);
  void cleanup();

public:
  TextureStore();
  ~TextureStore();
  bool init(SDL_Renderer*&);
};
