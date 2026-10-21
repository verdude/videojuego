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

const AnimationFrame&
AnimationClip::frame(size_t i) const
{
  return animation_frames.at(i);
}

uint64_t
AnimationClip::num_frames() const
{
  return animation_frames.size();
}
