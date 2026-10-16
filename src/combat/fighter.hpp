#pragma once

#include "fightercommand.hpp"
#include "move.hpp"

#include <memory>
#include <vector>

class FighterDefinition
{
  const std::unique_ptr<MoveDefinition> punch_;
  const std::unique_ptr<MoveDefinition> walk_;
  const std::unique_ptr<MoveDefinition> neutral_;
  const std::unique_ptr<MoveDefinition> block_;

public:
  FighterDefinition(std::unique_ptr<MoveDefinition> punch,
                    std::unique_ptr<MoveDefinition> walk,
                    std::unique_ptr<MoveDefinition> neutral,
                    std::unique_ptr<MoveDefinition> block)
    : punch_(std::move(punch))
    , walk_(std::move(walk))
    , neutral_(std::move(neutral))
    , block_(std::move(block))
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
