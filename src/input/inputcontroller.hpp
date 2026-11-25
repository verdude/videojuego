#pragma once

#include "../input/held.hpp"
#include "../input/input.hpp"
#include "../util/ringbuffer.hpp"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_scancode.h>
#include <unordered_map>
#include <vector>

#define HISTORY_LENGTH 32

class InputController
{
protected:
  std::vector<Input> commands;
  // TODO: pair Input with Facing direction
  RingBuffer<Input, HISTORY_LENGTH> history;
  Held held;

public:
  virtual ~InputController() = default;
  virtual void ingest(const std::vector<SDL_Event>&, uint64_t) = 0;
  virtual void reset() = 0;

  const std::vector<Input>& get_commands() const { return commands; }
  const RingBuffer<Input, HISTORY_LENGTH>& get_history() const
  {
    return history;
  }
  const Held& get_held() const { return held; }
};
