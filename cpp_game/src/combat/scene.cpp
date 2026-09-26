#include "scene.hpp"
#include "fightercommand.hpp"
#include <SDL3/SDL_scancode.h>
#include <array>
#include <iostream>

#include "../input/keyboardcontroller.hpp"
#include "fighter.hpp"

namespace battle_scene_detail {

std::array<FighterCommand, SDL_SCANCODE_COUNT>
buildPlayerKeymap()
{
  std::array<FighterCommand, SDL_SCANCODE_COUNT> playerKeymap{};
  playerKeymap.fill(None);
  playerKeymap[SDL_SCANCODE_A] = MoveLeft;
  playerKeymap[SDL_SCANCODE_D] = MoveRight;
  playerKeymap[SDL_SCANCODE_J] = Punch;
  return playerKeymap;
}

std::array<FighterCommand, SDL_SCANCODE_COUNT>
buildSecondaryKeymap()
{
  std::array<FighterCommand, SDL_SCANCODE_COUNT> otherKeymap{};
  otherKeymap.fill(None);
  otherKeymap[SDL_SCANCODE_LEFT] = MoveLeft;
  otherKeymap[SDL_SCANCODE_RIGHT] = MoveRight;
  otherKeymap[SDL_SCANCODE_KP_0] = Punch;
  return otherKeymap;
}

}

BattleScene
localBattle()
{
  return BattleScene(std::make_unique<KeyboardController>(
                       battle_scene_detail::buildPlayerKeymap()),
                     std::make_unique<KeyboardController>(
                       battle_scene_detail::buildSecondaryKeymap()));
}

void
BattleScene::start()
{
  std::cout << "start todo\n";
}

void
BattleScene::ingest(const std::vector<SDL_Event>& events)
{
  playerController->ingest(events);
  otherController->ingest(events);
}

void
BattleScene::update()
{
}

BattleScene
localBattle();
