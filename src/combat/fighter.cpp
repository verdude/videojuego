#include "fighter.hpp"

#include "fightercommand.hpp"

#include <iostream>
#include <vector>

MoveState FighterDefinition::punch() {
  return MoveState(punch_.get());
}

MoveState FighterDefinition::walk() {
  return MoveState(walk_.get());
}

MoveState FighterDefinition::neutral() {
  return MoveState(neutral_.get());
}

MoveState FighterDefinition::block() {
  return MoveState(block_.get());
}

void
Fighter::update(const std::vector<FighterCommand>& commands)
{
  for (auto c : commands) {
    std::cout << "Fighter [" << id << "] responding to command: " << c << "\n";
  }
}
