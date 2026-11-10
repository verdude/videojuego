#pragma once

#include <cstdint>
#include <string>

using Tick = uint64_t;

/**
 * The possible inputs. Count is just for convenience for getting the size of
 * the enum.
 */
enum Control
{
  Left,
  Right,
  Up,
  Down,
  One,
  Two,
  Three,
  Four,
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
  enum class InputState
  {
    Up,
    Down,
    Any,
  };
};
