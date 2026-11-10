#include "command.hpp"

CommandControl
command_control_from_string(const std::string& control)
{
  if (control == "back") {
    return CommandControl::Back;
  } else if (control == "forward") {
    return CommandControl::Forward;
  } else if (control == "up") {
    return CommandControl::Up;
  } else if (control == "down") {
    return CommandControl::Down;
  } else if (control == "one") {
    return CommandControl::One;
  } else if (control == "two") {
    return CommandControl::Two;
  } else if (control == "three") {
    return CommandControl::Three;
  } else if (control == "four") {
    return CommandControl::Four;
  }
  return CommandControl::Count;
}

void
Step::update(const std::string& control, bool down)
{
  const CommandControl id = command_control_from_string(control);
  if (CommandControl(id) >= CommandControl::Count) {
    throw std::runtime_error("Invalid control: " + control);
  }
  state[static_cast<size_t>(id)] = down ? Requirement::Down : Requirement::Up;
}

Control
resolve_control(CommandControl c, Facing f)
{
  switch (c) {
    case CommandControl::Back:
      return f == Facing::Left ? Control::Right : Control::Left;
    case CommandControl::Forward:
      return f == Facing::Right ? Control::Right : Control::Left;
    case CommandControl::Up:
      return Control::Up;
    case CommandControl::Down:
      return Control::Down;
    case CommandControl::One:
      return Control::One;
    case CommandControl::Two:
      return Control::Two;
    case CommandControl::Three:
      return Control::Three;
    case CommandControl::Four:
      return Control::Four;
    default:
      throw std::runtime_error(
        "Unhandled Control value: " + std::to_string(static_cast<int>(c)) +
        ", Facing value: " + std::to_string(static_cast<int>(f)) + ".");
  }
}

bool
Step::match(const Held& held, Facing facing) const
{
#pragma GCC unroll static_cast < size_t>(CommandControl::Count)
  for (size_t i = 0; i < static_cast<size_t>(CommandControl::Count); ++i) {
    if (state[i] == Requirement::Any) {
      continue;
    }
    const Control ctrl = resolve_control(CommandControl(i), facing);
    if ((state[i] == Requirement::Down) != held[ctrl].down) {
      return false;
    }
  }
  return true;
}
