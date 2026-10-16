#pragma once

#include "../animation/assetstore.hpp"
#include "../graphics/graphics.hpp"
#include "../input/inputcontroller.hpp"
#include "../input/keyboardcontroller.hpp"
#include "fighter.hpp"

#include <memory>
#include <vector>

class FightScene
{
private:
  Fighter player;
  Fighter other;

  std::unique_ptr<InputController> playerController;
  std::unique_ptr<InputController> otherController;

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

std::array<FighterCommand, SDL_SCANCODE_COUNT>
buildPlayerKeymap();

std::array<FighterCommand, SDL_SCANCODE_COUNT>
buildSecondaryKeymap();

} // namespace fight_scene_detail

FightScene localFight(std::shared_ptr<FighterDefinition>,
                      std::shared_ptr<FighterDefinition>);
