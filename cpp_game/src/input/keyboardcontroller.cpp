#include <SDL3/SDL_events.h>
#include <SDL3/SDL_log.h>

#include "keyboardcontroller.hpp"
#include "../combat/fightercommand.hpp"

void
KeyboardController::ingest(const std::vector<SDL_Event>& events)
{
  commands.clear();
  for (auto event : events) {
    if (event.type == SDL_EVENT_KEY_DOWN &&
        keymap[event.key.scancode] != FighterCommand::None) {
        SDL_Log("scancode: %s",
        SDL_GetScancodeName(event.key.scancode));
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
