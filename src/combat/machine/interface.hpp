#pragma once

#include "../../input/input.hpp"

#include <memory>
#include <vector>

class FighterStateMachine
{
public:
  virtual ~FighterStateMachine() = default;
  virtual std::unique_ptr<FighterStateMachine> transition(
    const std::vector<Input>&) = 0;
};
