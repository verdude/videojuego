#include "move.hpp"

#include "../animation/animationclip.hpp"

MoveDefinition::MoveDefinition(uint32_t damage_, AnimationClip* animation_)
  : damage(damage_)
  , animation(animation_)
{
}

const AnimationClip*
MoveDefinition::get_animation() const
{
  return animation;
}

uint64_t
MoveDefinition::total_frames() const
{
  return animation->num_frames();
}

MoveState::MoveState(MoveDefinition& move_)
  : move(move_)
  , current_frame(0)
  , started(false)
  , flip(false)
{
}

MoveState::MoveState(MoveDefinition& move_, bool flip_)
  : move(move_)
  , current_frame(0)
  , started(false)
  , flip(flip_)
{
}

const AnimationFrame&
MoveState::frame() const
{
  return move.get_animation()->frame(current_frame);
}

bool
MoveState::complete() const
{
  return move.total_frames() <= current_frame;
}

void
MoveState::tick()
{
  if (started) {
    ++current_frame;
  } else
    started = true;
}
