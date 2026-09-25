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

template <typename T>
Fighter<T>::Fighter(T controller_)
  : controller(controller_)
{
}
