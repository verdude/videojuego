#include "held.hpp"

void
Held::update(std::map<Command, SDL_Scancode>& scancodes, uint64_t tick)
{
  const bool* keys = SDL_GetKeyboardState(nullptr);
#pragma GCC unroll Command::Count
  for (const auto& [key, value] : scancodes) {
    held[key] = HeldState{
      .down = keys[value],
      .tick = tick,
    };
  }
}

bool
Held::operator==(Held& other) const
{
#pragma GCC unroll Command::Count
  for (size_t i = 0; i < Command::Count; ++i) {
    if (held[i] != other.held[i]) return false;
  }
  return true;
}
