#include "../input/action.hpp"
#include <SDL3/SDL_scancode.h>
#include <iostream>
#include <array>
#include "scene.hpp"

#include "../input/keyboardcontroller.hpp"
#include "fighter.hpp"

template <typename T>
BattleScene<T>::BattleScene() : player(0), other(0) {}

template <typename T>
void
BattleScene<T>::load()
{
  std::array<Action, SDL_SCANCODE_COUNT> playerKeymap{};
  playerKeymap.fill(None);
  playerKeymap[SDL_SCANCODE_A] = MoveLeft;
  playerKeymap[SDL_SCANCODE_D] = MoveRight;
  playerKeymap[SDL_SCANCODE_J] = Punch;
  player = Fighter(KeyboardController(playerKeymap));

  std::array<Action, SDL_SCANCODE_COUNT> otherKeymap{};
  otherKeymap.fill(None);
  otherKeymap[SDL_SCANCODE_LEFT] = MoveLeft;
  otherKeymap[SDL_SCANCODE_RIGHT] = MoveRight;
  otherKeymap[SDL_SCANCODE_KP_0] = Punch;
  other = Fighter(KeyboardController(otherKeymap));
}

template <typename T>
void
BattleScene<T>::start()
{
  std::cout << "start todo\n";
}
