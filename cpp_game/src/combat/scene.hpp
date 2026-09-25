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
