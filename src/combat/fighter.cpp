#include "fighter.hpp"

#include "../input/input.hpp"

#include <iostream>
#include <vector>

MoveState
FighterDefinition::punch(bool flip)
{
  return MoveState(punch_, flip);
}

MoveState
FighterDefinition::walk(bool flip)
{
  return MoveState(walk_, flip);
}

MoveState
FighterDefinition::neutral(bool flip)
{
  return MoveState(neutral_, flip);
}

MoveState
FighterDefinition::block(bool flip)
{
  return MoveState(block_, flip);
}

void
Fighter::update(const std::vector<Input>& commands,
                const std::array<HeldState, Command::Count>& held)
{
  current_move.tick();
  for (auto c : commands) {
    std::cout << "Fighter [" << id << "] responding to command: " << c.command
              << "\n";
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
