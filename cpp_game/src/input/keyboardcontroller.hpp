#pragma once

#include <array>
#include "inputcontroller.hpp"
#include "keyboardcontroller.hpp"
#include <vector>
#include "action.hpp"

class KeyboardController : public InputController
{
private:
  std::array<Action, SDL_SCANCODE_COUNT> keymap;
  std::vector<Action> commands;

public:
  KeyboardController(std::array<Action, SDL_SCANCODE_COUNT> k)
    : keymap(k), commands() {}

  void ingest(std::vector<SDL_Event>);
  void ingest(SDL_Event);
  void reset();
};
