#pragma once

#include "input.hpp"
#include "inputcontroller.hpp"
#include "keyboardcontroller.hpp"

#include <array>
#include <vector>

class KeyboardController : public InputController
{
private:
  std::array<Command, SDL_SCANCODE_COUNT> keymap;

public:
  KeyboardController(std::array<Command, SDL_SCANCODE_COUNT> k)
    : InputController()
    , keymap(k)
  {
  }

  void ingest(const std::vector<SDL_Event>&, uint64_t tick);
  void reset();
  void update_held_keys();
};
