#pragma once

#include "../animation/animationclip.hpp"

class MoveDefinition
{
  int startup_frames;
  int active_frames;
  int recovery_frames;
  int damage;
  AnimationClip* animation;

public:
  MoveDefinition(int, int, int, int, AnimationClip*);
  const AnimationClip* get_animation() const;
};

class MoveState
{
  MoveDefinition& move;
  size_t current_frame;

public:
  MoveState(MoveDefinition&);
  const AnimationFrame& frame() const;
};
