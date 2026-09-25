#pragma once

#include "../input/keyboardcontroller.hpp"

template <typename T>
class Fighter
{
private:
  T controller;

public:
  Fighter(T);
};
