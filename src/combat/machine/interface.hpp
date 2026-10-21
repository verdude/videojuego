#pragma once

class FighterStateMachine
{
public:
  virtual FighterCombatState() = default;
  virtual const FighterStateMachine transition(
    const std::vector<FighterCommand>&) = 0;
};
