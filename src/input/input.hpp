#pragma once

#include <cstdint>

using Tick = uint64_t;

enum Command
{
  Left,
  Right,
  Punch,
  Count,
  None,
};

enum class InputChange
{
  pressed,
  released
};

struct Input
{
  Command command;
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
