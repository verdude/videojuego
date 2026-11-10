#pragma once

#include "../input/inputcontroller.hpp"
#include "command.hpp"

#include <nlohmann/json.hpp>
#include <optional>
#include <string>

using json = nlohmann::json;

class CommandRecognizer
{
private:
  std::string config_path;
  std::vector<Command> commands;

public:
  CommandRecognizer(const char* p)
    : config_path(p)
    , commands()
  {
  }

  void load();
  std::optional<CommandEvent> recognize(const InputController&, Facing, Tick);
};
