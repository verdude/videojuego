#include "keyboardcontroller.hpp"

#include "input.hpp"

#include <SDL3/SDL_events.h>

void
KeyboardController::ingest(const std::vector<SDL_Event>& events, uint64_t tick)
{
  commands.clear();
  for (auto event : events) {
    if ((event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_KEY_UP) &&
        keymap[event.key.scancode] != Command::None && !event.key.repeat) {
      Command command = keymap[event.key.scancode];
      Input input = Input{
        .command = command,
        .change = event.type == SDL_EVENT_KEY_DOWN ? InputChange::pressed
                                                   : InputChange::released,
        .tick = tick,
      };
      commands.push_back(input);
      held.update(command, event.key.down, tick);
      // TODO: history
    }
  }
}

void
KeyboardController::reset()
{
  commands.clear();
}
