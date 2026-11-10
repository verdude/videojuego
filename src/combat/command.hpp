#pragma once

#include "../input/held.hpp"

#include <cstdint>
#include <string>
#include <vector>

using CommandId = uint16_t;
using Tick = uint64_t;

enum Requirement
{
  Any,
  Up,
  Down,
};

struct Step
{
  // TODO: trigger requirements, timing, etc.
  std::array<Requirement, Control::Count> button_state{};

  void update(const std::string& control, bool down)
  {
    button_state[control_from_string(control)] =
      down ? Requirement::Down : Requirement::Up;
  }

  bool match(const Held& held) const
  {
#pragma GCC unroll Control::Count
    for (size_t i = 0; i < Control::Count; ++i) {
      if (button_state[i] == Requirement::Any) {
        continue;
      }
      if ((button_state[i] == Requirement::Down) != held[i].down) {
        return false;
      }
    }
    return true;
  }
};

struct Command
{
  std::string name;
  std::vector<Step> steps;
};

/**
 * Represents a recognized command
 */
struct CommandEvent
{
  /**
   * Corresponds to the command in the command definitions
   * which is held in the CommandRecognizer.
   */
  CommandId command;
  Tick completedAt;
};
