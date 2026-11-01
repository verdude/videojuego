#pragma once

#include "../input/input.hpp"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_scancode.h>
#include <array>
#include <unordered_map>
#include <vector>

#define MAX_HISTORY_LENGTH 10

class InputController
{
protected:
  std::vector<Input> commands;
  // TODO: Move out and replace with array
  std::vector<Input> history;
  std::array<HeldState, Command::Count> held{};

public:
  virtual ~InputController() = default;
  virtual void ingest(const std::vector<SDL_Event>&, uint64_t) = 0;
  virtual void reset() = 0;
  virtual void update_held_keys(uint64_t) = 0;

  const std::vector<Input>& get_commands() const { return commands; }
  const std::array<HeldState, Command::Count>& get_held() const { return held; }
};
