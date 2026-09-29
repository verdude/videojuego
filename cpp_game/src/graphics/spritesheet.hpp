#pragma once

#include "animationclip.hpp"

#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class SpriteSheet
{
private:
  const char* path;
  std::unordered_map<const char*, AnimationClip> animations;

public:
  SpriteSheet(const char* p)
    : path(p)
    , animations()
  {
  }

  void load();
};
