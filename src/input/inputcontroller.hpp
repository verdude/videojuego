#pragma once

#include "../input/held.hpp"
#include "../input/input.hpp"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_scancode.h>
#include <unordered_map>
#include <vector>

#define MAX_HISTORY_LENGTH 10

class InputController
{
protected:
  std::vector<Input> commands;
  // TODO: Move out and replace with Array
  // TODO: pair Input with Facing direction
  std::vector<Input> history;
  Held held;

public:
  virtual ~InputController() = default;
  virtual void ingest(const std::vector<SDL_Event>&, uint64_t) = 0;
  virtual void reset() = 0;

  const std::vector<Input>& get_commands() const { return commands; }
  const std::vector<Input>& get_history() const { return history; }
  const Held& get_held() const { return held; }
};
