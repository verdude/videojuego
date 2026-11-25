#pragma once

#include "../animation/animationclip.hpp"
#include "../input/input.hpp"

#include <cstdint>
#include <optional>

class MoveDefinition
{
  uint32_t damage;
  AnimationClip* animation;
  // TODO: determine whether can update facing during move

public:
  MoveDefinition(uint32_t, AnimationClip*);
  const AnimationClip* get_animation() const;
  std::size_t total_frames() const;
  std::optional<MoveDefinition> next() const;
};

class MoveState
{
  MoveDefinition& move;
  std::size_t current_frame;
  double frame_millis;

public:
  MoveState(MoveDefinition&);
  MoveState(MoveDefinition&, bool);
  const AnimationFrame& frame() const;
  bool complete() const;
  bool can_auto_facing() const;
  void tick();
};
