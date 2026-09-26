#pragma once

#include "fightercommand.hpp"
#include <vector>

class Fighter
{
public:
  Fighter() {}
  void update(const std::vector<FighterCommand>&);
};
