#pragma once

#include <cstdint>
#include <string>

using Tick = uint64_t;

enum Control
{
  Left,
  Right,
  One,
  Count,
  None,
};

Control
control_from_string(const std::string&);

enum class InputChange
{
  pressed,
  released
};

struct Input
{
  Control command;
  InputChange change;
  Tick tick;
};

struct HeldState
{
  bool down = false;
  Tick tick;

  bool operator!=(const HeldState& other) const { return other.down != down; }
};

struct Chord
{
  enum InputState
  {
    Up,
    Down,
    Any,
  };
};
