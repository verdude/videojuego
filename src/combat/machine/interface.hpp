#pragma once

#include "../fightercommand.hpp"

#include <memory>
#include <vector>

class FighterStateMachine
{
public:
  virtual ~FighterStateMachine() = default;
  virtual std::unique_ptr<FighterStateMachine> transition(
    const std::vector<FighterCommand>&) = 0;
};
