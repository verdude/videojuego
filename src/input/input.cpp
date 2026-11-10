#include "input.hpp"

Control
control_from_string(const std::string& control)
{
  if (control == "left") {
    return Control::Left;
  } else if (control == "right") {
    return Control::Right;
  } else if (control == "one") {
    return Control::One;
  }
  return Control::None;
}
