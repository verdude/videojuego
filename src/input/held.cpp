#include "held.hpp"

void
Held::update(Control command, bool down, uint64_t tick)
{
  held[command] = HeldState{
    .down = down,
    .tick = tick,
  };
}

bool
Held::operator==(const Held& other) const
{
#pragma GCC unroll Control::Count
  for (size_t i = 0; i < Control::Count; ++i) {
    if (held[i] != other[i])
      return false;
  }
  return true;
}

const HeldState&
Held::operator[](size_t i) const
{
  return held[i];
}
