#pragma once

#include "input.hpp"
#include "inputcontroller.hpp"
#include "keyboardcontroller.hpp"

#include <array>
#include <map>
#include <vector>

class KeyboardController : public InputController
{
private:
  std::map<Command, SDL_Scancode> scancodes;
  std::array<Command, SDL_SCANCODE_COUNT> keymap{};

public:
  KeyboardController(std::map<Command, SDL_Scancode> k)
    : InputController()
    , scancodes(k)
  {
    keymap.fill(Command::None);
    for (const auto& [key, value] : scancodes) {
      keymap[value] = key;
    }
  }

  void ingest(const std::vector<SDL_Event>&, uint64_t);
  void reset();
  void update_held_keys(uint64_t);
};
