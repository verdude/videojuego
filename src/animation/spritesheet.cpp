#include "spritesheet.hpp"

void
SpriteSheet::load()
{
  std::ifstream f(path);
  json data = json::parse(f);
  for (auto tag : data["frameTags"]) {
    for (auto i = tag["from"].get<int>(); i < tag["to"].get<int>(); ++i) {
      auto clip = AnimationClip();
      animations.insert({});
    }
  }
}
