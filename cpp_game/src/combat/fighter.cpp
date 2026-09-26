#include "fighter.hpp"
#include "fightercommand.hpp"
#include <iostream>
#include <vector>

void
Fighter::update(const std::vector<FighterCommand>& commands)
{
  for (auto c : commands) {
    std::cout << "Fighter [" << id << "] responding to command: " << c << "\n";
  }
}
