#pragma once

#include "../animation/assetstore.hpp"
#include "../graphics/graphics.hpp"
#include "../input/inputcontroller.hpp"
#include "../input/keyboardcontroller.hpp"
#include "commandrecognizer.hpp"
#include "fighter.hpp"
#include "scene.hpp"

#include <memory>
#include <vector>

class FightScene
{
private:
  uint64_t sim_tick = 0;

  Fighter player;
  Fighter other;

  std::unique_ptr<InputController> player_controller;
  std::unique_ptr<InputController> other_controller;

  CommandRecognizer cmd_recognizer;

public:
  FightScene(std::unique_ptr<InputController>,
             std::unique_ptr<InputController>,
             std::shared_ptr<FighterDefinition>,
             std::shared_ptr<FighterDefinition>);

  void start();
  void ingest(const std::vector<SDL_Event>&);
  void update();
  void render(Graphics&, AssetStore&);
};

namespace fight_scene_detail {

std::map<Control, SDL_Scancode>
buildPlayerKeymap();

std::map<Control, SDL_Scancode>
buildSecondaryKeymap();

struct FighterRender
{
  const AnimationFrame& frame;
  Rect destination;
};

const FighterRender
texture_destination(Fighter& fighter);

} // namespace fight_scene_detail

FightScene localFight(std::shared_ptr<FighterDefinition>,
                      std::shared_ptr<FighterDefinition>);
