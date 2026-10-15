#pragma once

#include <vector>

struct AnimationFrame
{
  int x, y, w, h;
};

class AnimationClip
{
private:
  const char* name;
  std::vector<AnimationFrame> animation_frames;

public:
  AnimationClip(const char*);
};
