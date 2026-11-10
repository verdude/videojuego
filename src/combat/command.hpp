#pragma once

#include "../input/held.hpp"

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>

using CommandId = uint16_t;
using Tick = uint64_t;

/**
 * The direction the fighter is facing.
 * Helps determine if a Left/Right input maps to Forward or Back.
 */
enum class Facing
{
  Left,
  Right
};

/**
 * Logical Commands. Differs from the raw controls on Forward/Back vs
 * Left/Right. These are the basic components of move inputs. Raw Controls are
 * converted into these using context from the fighter when necessary.
 */
enum class CommandControl
{
  Forward,
  Back,
  Up,
  Down,
  One,
  Two,
  Three,
  Four,
  Count,
};

/**
 * For parsing, just called on startup.
 */
CommandControl
command_control_from_string(const std::string&);

/**
 * Utility function to map a CommandControl -> Control.
 */
Control resolve_control(CommandControl, Facing);

/**
 * The required button state for a particular command.
 */
enum class Requirement
{
  Any,
  Up,
  Down,
};

/**
 * A single step in a command. Created once and used as a reference.
 */
struct Step
{
  // TODO: trigger requirements, timing, etc.
  /**
   * The complete mapping for this step.
   */
  std::array<Requirement, static_cast<size_t>(CommandControl::Count)> state{};

  /**
   * Update the definition of a single Command.
   * Called at most once per command on load.
   *
   * @param string the string representation of the CommandControl member
   * @param bool whether the button must be pressed or not.
   */
  void update(const std::string&, bool);

  /**
   * Determine whether currently held buttons match this step's requirements.
   */
  bool match(const Held&, Facing, Tick) const;
};

/**
 * A possible command with a non-zero sequence of steps defined.
 */
struct Command
{
  std::string name;
  std::vector<Step> steps;
};

/**
 * Represents a matched command.
 */
struct CommandEvent
{
  /**
   * Corresponds to the index of the command in the command definitions which is
   * held in the CommandRecognizer.
   */
  CommandId command;

  /**
   * The tick during which the full command was completed and matched.
   */
  Tick completedAt;
};
