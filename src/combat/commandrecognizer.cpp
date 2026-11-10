#include "commandrecognizer.hpp"

#include <fstream>
#include <stdexcept>

void
CommandRecognizer::load()
{
  std::ifstream f(config_path);
  if (!f) {
    throw std::runtime_error("Failed to open command config " + config_path);
  }
  const json data = json::parse(f);
  for (const auto& command : data) {
    std::vector<Step> steps(command.at("steps").size());
    size_t i = 0;
    for (const auto& step : command.at("steps")) {
      steps[i] = Step{};
      for (const auto& [key, val] : step.items()) {
        steps[i].update(key, val);
      }
      ++i;
    }
    commands.push_back(Command{
      .name = command.at("name").get<std::string>(),
      .steps = steps,
    });
  }
}

std::optional<CommandEvent>
CommandRecognizer::recognize(const InputController& ctlr,
                             const Fighter& fighter,
                             Tick tick)
{
  for (CommandId i = 0; i < commands.size(); ++i) {
    bool match = true;
    for (const auto& step : commands[i].steps) {
      if (!step.match(ctlr.get_held())) {
        match = false;
        break;
      }
    }
    if (!match) {
      continue;
    } else {
      return CommandEvent{
        .command = i,
        .completedAt = tick,
      };
    }
  }
  return std::nullopt;
}
