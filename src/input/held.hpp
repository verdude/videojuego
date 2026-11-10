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
  std::array<HeldState, Control::Count> held{};

public:
  /**
   * Update a currently held button
   *
   * @param Control the pressed button
   * @param bool whether the button is pressed
   * @param uint64_t tick number
   */
  void update(Control, bool, uint64_t);

  /**
   * Return true if the same set of commands are held in both objects.
   */
  bool operator==(const Held&) const;

  /**
   * Get the state of a particular Control
   */
  const HeldState& operator[](size_t) const;
};
