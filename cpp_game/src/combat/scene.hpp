#pragma once

#include "fighter.hpp"
#include "../input/inputcontroller.hpp"
#include "../input/keyboardcontroller.hpp"

template <typename T = KeyboardController>
class BattleScene
{
private:
  Fighter<KeyboardController> player;
  Fighter<T> other;

public:
  BattleScene();
  void load();
  void start();
};

inline
std::array<Action, SDL_SCANCODE_COUNT> buildPlayerKeymap() {
  std::array<Action, SDL_SCANCODE_COUNT> playerKeymap{};
  playerKeymap.fill(None);
  playerKeymap[SDL_SCANCODE_A] = MoveLeft;
  playerKeymap[SDL_SCANCODE_D] = MoveRight;
  playerKeymap[SDL_SCANCODE_J] = Punch;
  return playerKeymap;
}

inline
std::array<Action, SDL_SCANCODE_COUNT> buildSecondaryKeymap() {
  std::array<Action, SDL_SCANCODE_COUNT> otherKeymap{};
  otherKeymap.fill(None);
  otherKeymap[SDL_SCANCODE_LEFT] = MoveLeft;
  otherKeymap[SDL_SCANCODE_RIGHT] = MoveRight;
  otherKeymap[SDL_SCANCODE_KP_0] = Punch;
  return otherKeymap;
}

template <typename T>
BattleScene<T>::BattleScene() : player(KeyboardController(buildPlayerKeymap())), other(KeyboardController(buildSecondaryKeymap()))
{ }

template <typename T>
void
BattleScene<T>::start()
{
  std::cout << "start todo\n";
}
