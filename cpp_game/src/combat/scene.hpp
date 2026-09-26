#pragma once

#include "../input/inputcontroller.hpp"
#include "../input/keyboardcontroller.hpp"
#include "fighter.hpp"
#include <iostream>
#include <memory>
#include <vector>

class FightScene
{
private:
  Fighter player;
  Fighter other;

  std::unique_ptr<InputController> playerController;
  std::unique_ptr<InputController> otherController;

public:
  FightScene(std::unique_ptr<InputController> c1,
             std::unique_ptr<InputController> c2)
    : player()
    , other()
    , playerController(std::move(c1))
    , otherController(std::move(c2))
  {
  }

  void start();
  void ingest(const std::vector<SDL_Event>&);
  void update();
};

namespace fight_scene_detail {

std::array<FighterCommand, SDL_SCANCODE_COUNT>
buildPlayerKeymap();

std::array<FighterCommand, SDL_SCANCODE_COUNT>
buildSecondaryKeymap();

} // namespace fight_scene_detail

FightScene
localFight();
