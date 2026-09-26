#pragma once

#include "../combat/fightercommand.hpp"
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_scancode.h>
#include <unordered_map>
#include <vector>

class InputController
{
public:
  virtual ~InputController() = default;
  virtual void ingest(std::vector<SDL_Event>&) = 0;
  virtual void ingest(SDL_Event) = 0;
  virtual void reset() = 0;
};
