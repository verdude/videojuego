#pragma once

class FighterStateMachine
{
public:
  virtual FighterCombatState() = default;
  virtual const FighterStateMachine transition(
    const std::vector<Command>&) = 0;
};
