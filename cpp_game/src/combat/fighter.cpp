#include "fighter.hpp"
#include "fightercommand.hpp"
#include <vector>
#include <iostream>

void
Fighter::update(const std::vector<FighterCommand>& commands)
{
  for (auto c : commands) {
    std::cout << "Fighter responding to command: " << c << "\n";
  }
}
