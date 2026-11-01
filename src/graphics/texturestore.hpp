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

struct LoadTextureResult
{
  bool success;
  // TODO: return pointer to reduce complexity
  size_t key;
};

class TextureStore
{
private:
  std::unordered_map<size_t, const Texture> textures;

  void cleanup();

public:
  TextureStore();
  ~TextureStore();
  TextureStore(const TextureStore&) = delete;
  TextureStore& operator=(const TextureStore&) = delete;
  LoadTextureResult load_from_file(const char*, SDL_Renderer*);
  const Texture* get(size_t) const;
};
