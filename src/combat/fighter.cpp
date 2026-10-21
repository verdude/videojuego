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
  current_move.tick();
  for (auto c : commands) {
    std::cout << "Fighter [" << id << "] responding to command: " << c << "\n";
    switch (c) {
      case FighterCommand::MoveLeft:
        current_move = fighter_definition.walk(true);
        break;
      case FighterCommand::MoveRight:
        current_move = fighter_definition.walk(false);
        break;
      case FighterCommand::Punch:
        current_move = fighter_definition.punch();
        break;
      case FighterCommand::Block:
        current_move = fighter_definition.block();
        break;
      default:
        std::cerr << "got NONE command\n";
        break;
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
