#pragma once

#include "fightercommand.hpp"
#include "move.hpp"

#include <memory>
#include <vector>

class FighterDefinition
{
  MoveDefinition punch_;
  MoveDefinition walk_;
  MoveDefinition neutral_;
  MoveDefinition block_;

public:
  FighterDefinition(MoveDefinition punch,
                    MoveDefinition walk,
                    MoveDefinition neutral,
                    MoveDefinition block)
    : punch_(punch)
    , walk_(walk)
    , neutral_(neutral)
    , block_(block)
  {
  }

  MoveState neutral();
  MoveState punch();
  MoveState walk();
  MoveState block();
};

class Fighter
{
private:
  const int id;
  const std::shared_ptr<FighterDefinition> fighter_definition;

  MoveState current_move;

public:
  Fighter(const int n, std::shared_ptr<FighterDefinition> who)
    : id(n)
    , fighter_definition(who)
    , current_move(who->neutral())
  {
  }
  void update(const std::vector<FighterCommand>&);
};
