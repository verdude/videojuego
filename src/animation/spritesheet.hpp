#pragma once

#include "../graphics/texturestore.hpp"
#include "animationclip.hpp"

#include <fstream>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>

using json = nlohmann::json;

class SpriteSheet
{
private:
  std::string path;
  std::unordered_map<std::string, AnimationClip> animations;

public:
  SpriteSheet(const char* p)
    : path(p)
    , animations()
  {
  }

  void load(const Texture*);
  AnimationClip* get_animation(std::string);
  const std::string& get_path() const;
};
