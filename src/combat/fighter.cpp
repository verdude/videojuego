#include "fighter.hpp"

#include "fightercommand.hpp"

#include <iostream>
#include <vector>

MoveState
FighterDefinition::punch()
{
  return MoveState(punch_);
}

MoveState
FighterDefinition::walk()
{
  return MoveState(walk_);
}

MoveState
FighterDefinition::neutral()
{
  return MoveState(neutral_);
}

MoveState
FighterDefinition::block()
{
  return MoveState(block_);
}

void
Fighter::update(const std::vector<FighterCommand>& commands)
{
  for (auto c : commands) {
    std::cout << "Fighter [" << id << "] responding to command: " << c << "\n";
  }
}

const AnimationFrame&
Fighter::frame() const
{
  return current_move.frame();
}

const Vec2&
Fighter::get_anchor() const
{
  return anchor;
}
