#include "fighter.hpp"

#include "fightercommand.hpp"

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
Fighter::update(const std::vector<FighterCommand>& commands)
{
  current_move.tick();
  for (auto c : commands) {
    std::cout << "Fighter [" << id << "] responding to command: " << c << "\n";
    // TODO: Implement command handling logic here
    if (c == FighterCommand::Punch) {
      // TODO: Punch
      std::cout << "Fighter [" << id << "] executing PUNCH move\n";
    } else if (c == FighterCommand::MoveLeft) {
      // Move LEFT
      std::cout << "Fighter [" << id << "] executing MOVE LEFT\n";
      this->anchor.x -= 5; // Move left by 5 units
    } else if (c == FighterCommand::MoveRight) {
      // Move RIGHT
      std::cout << "Fighter [" << id << "] executing MOVE RIGHT\n";
      this->anchor.x += 5; // Move right by 5 units
    } else if (c == FighterCommand::Block) {
      // Block
      std::cout << "Fighter [" << id << "] executing BLOCK move\n";
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
