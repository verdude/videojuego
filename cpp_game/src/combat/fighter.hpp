#pragma once

#include "fightercommand.hpp"
#include <vector>

class Fighter
{
private:
  const int id;
public:
  Fighter(const int n) : id(n) {}
  void update(const std::vector<FighterCommand>&);
};
