#include "fighter.hpp"

#include "../input/held.hpp"
#include "../input/input.hpp"

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
Fighter::advance_move_time()
{
  current_move.tick();
}

void
Fighter::update_facing(bool face_right)
{
  if (current_move.can_auto_facing()) {
    facing = face_right ? Facing::Right : Facing::Left;
  }
}

void
Fighter::update(const std::vector<Input>& commands, const Held& held)
{
  constexpr int movement_per_tick = 1;
  const int direction = static_cast<int>(held[Control::Right].down) -
                        static_cast<int>(held[Control::Left].down);
  anchor.x += direction * movement_per_tick;

  for (const auto& input : commands) {
    if (input.change != InputChange::pressed) {
      continue;
    }

    if (input.command == Control::One) {
      // TODO: Punch
      std::cout << "Fighter [" << id << "] executing PUNCH move\n";
    }
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
