#pragma once

#include "../geometry/types.hpp"
#include "../input/held.hpp"
#include "../input/input.hpp"
#include "move.hpp"
#include "command.hpp"

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
    : punch_(std::move(punch))
    , walk_(std::move(walk))
    , neutral_(std::move(neutral))
    , block_(std::move(block))
  {
  }

  MoveState neutral(bool = false);
  MoveState punch(bool = false);
  MoveState walk(bool = false);
  MoveState block(bool = false);
};

class Fighter
{
private:
  const int id;
  Vec2 anchor;
  Facing facing;
  const std::shared_ptr<const FighterDefinition> fighter_definition;

  MoveState current_move;

public:
  Fighter(const int n,
          Vec2 position,
          Facing f,
          std::shared_ptr<FighterDefinition> who)
    : id(n)
    , anchor(position)
    , facing(f)
    , fighter_definition(who)
    , current_move(who->neutral())
  {
  }

  /**
   * Just increments the animation frames for the fighters.
   * This allows updating facing before changing the move
   * according to inputs/completion.
   */
  void advance_move_time();

  /**
   * @param vector<Input> the inputs for the fighter
   * @param Held the currently held buttons
   */
  void update(const std::vector<Input>&, const Held&);

  /**
   * Update the facing direction for the fighter.
   * Flips the animation frames for the current animation.
   *
   * @param bool whether the player should face right.
   */
  void update_facing(bool);
  Facing get_facing() const { return facing; }

  const AnimationFrame& frame() const;
  const Vec2& get_anchor() const;
};
