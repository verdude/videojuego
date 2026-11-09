#pragma once

#include "../input/inputcontroller.hpp"
#include "../combat/fighter.hpp"

class CommandRecognizer {
public:
  CommandRecognizer() = delete;
  CommandRecognizer(CommandRecognizer&) = delete;
  CommandRecognizer(CommandRecognizer&&) = delete;

  //static CommandEvent recognize(const InputController&, const Fighter&);
};
