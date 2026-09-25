#include "../input/inputcontroller.hpp"
#include "fighter.hpp"

template <typename T>
Fighter<T>::Fighter(T controller)
  : controller(controller)
{
}
