#include "move.hpp"

#include "../animation/animationclip.hpp"

#include <algorithm>
#include <stdexcept>

MoveDefinition::MoveDefinition(std::uint32_t damage_, AnimationClip* animation_)
  : damage(damage_)
  , animation(animation_)
{
  if (animation == nullptr || animation->num_frames() == 0) {
    throw std::invalid_argument("A move requires a nonempty animation");
  }
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

std::optional<MoveDefinition>
MoveDefinition::next() const
{
  return std::nullopt;
}

MoveState::MoveState(MoveDefinition& move_)
  : move(move_)
  , current_frame(0)
  , frame_millis(0)
  , flip(false)
{
}

MoveState::MoveState(MoveDefinition& move_, bool flip_)
  : move(move_)
  , current_frame(0)
  , frame_millis(0)
  , flip(flip_)
{
}

const AnimationFrame&
MoveState::frame() const
{
  // Clamp the index to the last valid frame so completed moves keep their final
  // pose. Otherwise, we accidentally return an invalid frame, which crashes.
  return move.get_animation()->frame(
    std::min(current_frame, move.total_frames() - 1));
}

bool
MoveState::complete() const
{
  return move.total_frames() <= current_frame;
}

void
MoveState::tick()
{
  // Stop at completion so repeated ticks cannot overflow the frame counter.
  if (complete()) {
    frame_millis = 0;
    current_frame = 0;
    return;
  }

  // TODO: no magic
  frame_millis += 1000.0 / 60.0;
  auto duration = frame().duration;
  if (frame_millis >= duration) {
    ++current_frame;
    frame_millis -= duration;
  }
}
