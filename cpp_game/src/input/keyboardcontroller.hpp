#pragma once

#include "../combat/fightercommand.hpp"
#include "inputcontroller.hpp"
#include "keyboardcontroller.hpp"
#include <array>
#include <vector>

class KeyboardController : public InputController
{
private:
  std::array<FighterCommand, SDL_SCANCODE_COUNT> keymap;

public:
  KeyboardController(std::array<FighterCommand, SDL_SCANCODE_COUNT> k)
    : InputController()
    , keymap(k)
  {
  }

  void ingest(const std::vector<SDL_Event>&);
  void ingest(SDL_Event);
  void reset();
};
