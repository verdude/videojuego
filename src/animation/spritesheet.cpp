#include "spritesheet.hpp"

#include "animationclip.hpp"
#include "../graphics/texturestore.hpp"

#include <stdexcept>
#include <string>

void
SpriteSheet::load(Texture* texture)
{
  std::ifstream f(path);
  if (!f) {
    throw std::runtime_error("Failed to open spritesheet " + path);
  }
  json data = json::parse(f);
  for (const auto& tag : data.at("meta").at("frameTags")) {
    std::string name = tag.at("name").get<std::string>();
    auto clip = AnimationClip(name.c_str());
    for (auto i = tag.at("from").get<std::size_t>();
         i <= tag.at("to").get<std::size_t>();
         ++i) {
      const auto& frame = data.at("frames").at(i);
      clip.insert(AnimationFrame{
        .x = frame.at("frame").at("x"),
        .y = frame.at("frame").at("y"),
        .w = frame.at("frame").at("w"),
        .h = frame.at("frame").at("h"),
        .sourceW = frame.at("sourceSize").at("w"),
        .sourceH = frame.at("sourceSize").at("h"),
        .spriteSourceX = frame.at("spriteSourceSize").at("x"),
        .spriteSourceY = frame.at("spriteSourceSize").at("y"),
        .spriteSourceW = frame.at("spriteSourceSize").at("w"),
        .spriteSourceH = frame.at("spriteSourceSize").at("h"),
        .texture = texture,
      });
    }
    animations.insert({ name, clip });
  }
}

AnimationClip*
SpriteSheet::get_animation(std::string name)
{
  auto animation = animations.find(name);
  return animation != animations.end() ? &animation->second : nullptr;
}
