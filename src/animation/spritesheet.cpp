#include "spritesheet.hpp"

#include "../graphics/texturestore.hpp"
#include "animationclip.hpp"

#include <stdexcept>
#include <string>

void
SpriteSheet::load(const Texture* texture)
{
  std::ifstream f(path);
  if (!f) {
    throw std::runtime_error("Failed to open spritesheet " + path);
  }
  json data = json::parse(f);
  const auto& slices = data.at("meta").at("slices");
  if (slices.size() != 1) {
    throw std::runtime_error("Expected one anchor slice in spritesheet " +
                             path);
  }
  const auto& keys = slices.at(0).at("keys");
  if (keys.size() != 1 || keys.at(0).at("frame").get<int>() != 0) {
    throw std::runtime_error(
      "Expected one anchor slice key at frame 0 in spritesheet " + path);
  }
  const auto& slice = keys.at(0);
  const Vec2 pivot{
    slice.at("bounds").at("x").get<int>() +
      slice.at("pivot").at("x").get<int>(),
    slice.at("bounds").at("y").get<int>() +
      slice.at("pivot").at("y").get<int>(),
  };

  for (const auto& tag : data.at("meta").at("frameTags")) {
    std::string name = tag.at("name").get<std::string>();
    auto clip = AnimationClip(name.c_str());
    for (auto i = tag.at("from").get<std::size_t>();
         i <= tag.at("to").get<std::size_t>();
         ++i) {
      const auto& frame = data.at("frames").at(i);
      const auto& sprite_source = frame.at("spriteSourceSize");
      clip.insert(AnimationFrame{
        .x = frame.at("frame").at("x"),
        .y = frame.at("frame").at("y"),
        .w = frame.at("frame").at("w"),
        .h = frame.at("frame").at("h"),
        .sourceW = frame.at("sourceSize").at("w"),
        .sourceH = frame.at("sourceSize").at("h"),
        .spriteSourceX = sprite_source.at("x"),
        .spriteSourceY = sprite_source.at("y"),
        .spriteSourceW = sprite_source.at("w"),
        .spriteSourceH = sprite_source.at("h"),
        .drawOffset = { sprite_source.at("x").get<int>() - pivot.x,
                        sprite_source.at("y").get<int>() - pivot.y },
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

const std::string&
SpriteSheet::get_path() const
{
  return path;
}
