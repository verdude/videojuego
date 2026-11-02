#pragma once

#include "../geometry/types.hpp"
#include "../graphics/texturestore.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

struct AnimationFrame
{
  int x, y, w, h, sourceW, sourceH, spriteSourceX, spriteSourceY, spriteSourceW,
    spriteSourceH, duration;
  Vec2 drawOffset;
  const Texture* texture;
};

class AnimationClip
{
private:
  std::string name;
  std::vector<AnimationFrame> animation_frames;

public:
  AnimationClip(const char*);
  void insert(AnimationFrame);
  const AnimationFrame& frame(std::size_t) const;
  std::size_t num_frames() const;
};
