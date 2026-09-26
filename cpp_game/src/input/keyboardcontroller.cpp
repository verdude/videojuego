#include <SDL3/SDL_events.h>

#include "../combat/fightercommand.hpp"
#include "keyboardcontroller.hpp"

void
KeyboardController::ingest(const std::vector<SDL_Event>& events)
{
  commands.clear();
  for (auto event : events) {
    if (event.type == SDL_EVENT_KEY_DOWN &&
        keymap[event.key.scancode] != FighterCommand::None) {
      commands.push_back(keymap[event.key.scancode]);
    }
  }
}

void
KeyboardController::ingest(SDL_Event event)
{
  if (event.type == SDL_EVENT_KEY_DOWN &&
      keymap[event.key.scancode] != FighterCommand::None) {
    commands.push_back(keymap[event.key.scancode]);
  }
}

void
KeyboardController::reset()
{
  commands.clear();
}
