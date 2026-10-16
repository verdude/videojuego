#pragma once

#include "../graphics/texturestore.hpp"

#include <string>
#include <vector>

struct AnimationFrame
{
  int x, y, w, h, sourceW, sourceH, spriteSourceX, spriteSourceY, spriteSourceW,
    spriteSourceH;
  Texture* texture;
};

class AnimationClip
{
private:
  std::string name;
  std::vector<AnimationFrame> animation_frames;

public:
  AnimationClip(const char*);
  void insert(AnimationFrame);
  const AnimationFrame& frame(size_t) const;
};
