#pragma once

#include "../animation/animationclip.hpp"

class MoveDefinition
{
  uint32_t startup_frames;
  uint32_t active_frames;
  uint32_t recovery_frames;
  uint32_t damage;
  bool loop;
  AnimationClip* animation;

public:
  MoveDefinition(uint32_t, uint32_t, uint32_t, uint32_t, bool, AnimationClip*);
  const AnimationClip* get_animation() const;
  int total_frames() const;
};

class MoveState
{
  MoveDefinition& move;
  size_t current_frame;
  bool started;
  bool flip;

public:
  MoveState(MoveDefinition&, bool);
  const AnimationFrame& frame() const;
  bool complete() const;
  void tick();
};
