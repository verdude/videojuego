#pragma once

#include "../combat/fightercommand.hpp"
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_scancode.h>
#include <unordered_map>
#include <vector>

class InputController
{
  private:
    std::vector<FighterCommand> commands;
public:
  virtual ~InputController() = default;
  virtual void ingest(const std::vector<SDL_Event>&) = 0;
  virtual void ingest(SDL_Event) = 0;
  virtual void reset() = 0;

  const std::vector<FighterCommand>& get_commands() const {
    return commands;
  }
};
