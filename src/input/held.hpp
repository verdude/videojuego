#pragma once

#include "input.hpp"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_scancode.h>
#include <array>
#include <cstdint>
#include <map>

class Held
{
private:
  std::array<HeldState, Command::Count> held{};

public:
  /**
   * Update all the currently held keys
   *
   * @param Map of Command -> Scancode. Reverse keymap. SDL Specific.
   * @param uint64_t tick number
   */
  void update(Command, bool, uint64_t);

  /**
   * Return true if the same set of commands are held in both objects.
   */
  bool operator==(const Held&) const;
  const HeldState& operator[](size_t) const;
};
