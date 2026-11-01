#pragma once

#include "../input/input.hpp"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_scancode.h>
#include <unordered_map>
#include <vector>
#include <array>

class InputController
{
protected:
  std::vector<Input> commands;
  // TODO: Move out
  std::vector<Input> history;
  std::array<HeldState, static_cast<size_t>(Command::Count)> held{};

public:
  virtual ~InputController() = default;
  virtual void ingest(const std::vector<SDL_Event>&, uint64_t) = 0;
  virtual void reset() = 0;
  virtual void update_held_keys(uint64_t) = 0;

  const std::vector<Input>& get_commands() const { return commands; }
};
