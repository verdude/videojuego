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
};

class MoveState
{
  MoveDefinition& move;
  int current_frame;

public:
  MoveState(MoveDefinition&);
};
