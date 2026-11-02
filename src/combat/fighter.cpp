#include "fighter.hpp"

#include "../input/held.hpp"
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
Fighter::update(const std::vector<Input>& commands, const Held& held)
{
  constexpr int movement_per_tick = 5;
  const int direction = static_cast<int>(held[Command::Right].down) -
                        static_cast<int>(held[Command::Left].down);
  anchor.x += direction * movement_per_tick;

  for (const auto& input : commands) {
    if (input.change != InputChange::pressed) {
      continue;
    }

    if (input.command == Command::Punch) {
      // TODO: Punch
      std::cout << "Fighter [" << id << "] executing PUNCH move\n";
    }
  }
  current_move.tick();
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
