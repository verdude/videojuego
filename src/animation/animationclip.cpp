#include "animationclip.hpp"

AnimationClip::AnimationClip(const char* n)
  : name(n)
  , animation_frames()
{
}

void
AnimationClip::insert(AnimationFrame frame)
{
  animation_frames.push_back(frame);
}
