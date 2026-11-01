#pragma once

#include <cstdint>

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

using Tick = uint64_t;

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
};
