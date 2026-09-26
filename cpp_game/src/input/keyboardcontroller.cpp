#include "keyboardcontroller.hpp"
#include "../combat/fightercommand.hpp"

void
KeyboardController::ingest(const std::vector<SDL_Event>& events)
{
  commands.clear();
  for (auto event : events) {
    if (keymap[event.key.scancode] != FighterCommand::None) {
      commands.push_back(keymap[event.key.scancode]);
    }
  }
}

void
KeyboardController::ingest(SDL_Event event)
{
  if (keymap[event.key.scancode] != FighterCommand::None) {
    commands.push_back(keymap[event.key.scancode]);
  }
}

void
KeyboardController::reset()
{
  commands.clear();
}
