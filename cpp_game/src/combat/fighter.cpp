#include "../input/inputcontroller.hpp"
#include "fighter.hpp"

template <typename T>
Fighter<T>::Fighter(T controller_)
  : controller(controller_)
{
}
