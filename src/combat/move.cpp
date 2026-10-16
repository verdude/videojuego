#include "move.hpp"

#include "../animation/animationclip.hpp"

MoveDefinition::MoveDefinition(int startup_frames_,
                               int active_frames_,
                               int recovery_frames_,
                               int damage_,
                               AnimationClip* animation_)
  : startup_frames(startup_frames_)
  , active_frames(active_frames_)
  , recovery_frames(recovery_frames_)
  , damage(damage_)
  , animation(animation_)
{
}

MoveState::MoveState(MoveDefinition& move_)
  : move(move_)
  , current_frame(0)
{
}
