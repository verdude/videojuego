#include "keyboardcontroller.hpp"

#include "input.hpp"

#include <SDL3/SDL_events.h>

void
KeyboardController::ingest(const std::vector<SDL_Event>& events, uint64_t tick)
{
  commands.clear();
  for (auto event : events) {
    if ((event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_KEY_UP) &&
        keymap[event.key.scancode] != Command::None) {
      commands.push_back(
        Input{
          .command = keymap[event.key.scancode],
          .change = event.type == SDL_EVENT_KEY_DOWN ? InputChange::pressed : InputChange::released,
          .tick = tick,
        }
      );
    }
  }
}

void
KeyboardController::update_held_keys() {
  const bool* keys = SDL_GetKeyboardState(nullptr);
  held[Command::Left] = keys[keymap[Command::Left]];
}

void
KeyboardController::reset()
{
  commands.clear();
}
