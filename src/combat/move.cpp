#include "move.hpp"

#include "../animation/animationclip.hpp"

MoveDefinition::MoveDefinition(uint32_t startup_frames_,
                               uint32_t active_frames_,
                               uint32_t recovery_frames_,
                               uint32_t damage_,
                               bool loop_,
                               AnimationClip* animation_)
  : startup_frames(startup_frames_)
  , active_frames(active_frames_)
  , recovery_frames(recovery_frames_)
  , damage(damage_)
  , loop(loop_)
  , animation(animation_)
{
}

const AnimationClip*
MoveDefinition::get_animation() const
{
  return animation;
}

uint32_t
MoveDefinition::total_frames() const
{
  return startup_frames + active_frames + recovery_frames;
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
