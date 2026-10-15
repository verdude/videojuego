#include <string>

#include "spritesheet.hpp"

#include "animationclip.hpp"

void
SpriteSheet::load()
{
  std::ifstream f(path);
  json data = json::parse(f);
  for (auto tag : data["frameTags"]) {
    auto clip = AnimationClip(path);
    std::string name = tag["name"].get<std::string>();
    for (auto i = tag["from"].get<unsigned long>(); i < tag["to"].get<unsigned long>(); ++i) {
      auto frame = data["frames"][i];
      clip.insert(AnimationFrame{
        .x = frame["frame"]["x"],
        .y = frame["frame"]["y"],
        .w = frame["frame"]["w"],
        .h = frame["frame"]["h"],
        .sourceW = frame["frame"]["sourceSize"]["w"],
        .sourceH = frame["frame"]["sourceSize"]["h"],
        .spriteSourceX = frame["frame"]["spriteSourceSize"]["x"],
        .spriteSourceY = frame["frame"]["spriteSourceSize"]["y"],
        .spriteSourceW = frame["frame"]["spriteSourceSize"]["w"],
        .spriteSourceH = frame["frame"]["spriteSourceSize"]["h"],
      });
    }
    animations.insert({name, clip});
  }
}
