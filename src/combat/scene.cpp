#include "scene.hpp"

#include "../geometry/types.hpp"
#include "../input/input.hpp"
#include "../input/keyboardcontroller.hpp"
#include "fighter.hpp"

#include <SDL3/SDL_scancode.h>
#include <iostream>
#include <map>

namespace fight_scene_detail {

std::map<Command, SDL_Scancode>
buildPlayerKeymap()
{
  std::map<Command, SDL_Scancode> playerKeymap{};
  playerKeymap[Command::Left] = SDL_SCANCODE_A;
  playerKeymap[Command::Right] = SDL_SCANCODE_D;
  playerKeymap[Command::Punch] = SDL_SCANCODE_J;
  return playerKeymap;
}

std::map<Command, SDL_Scancode>
buildSecondaryKeymap()
{
  std::map<Command, SDL_Scancode> otherKeymap{};
  otherKeymap[Command::Left] = SDL_SCANCODE_LEFT;
  otherKeymap[Command::Right] = SDL_SCANCODE_RIGHT;
  otherKeymap[Command::Punch] = SDL_SCANCODE_KP_0;
  return otherKeymap;
}

const FighterRender
texture_destination(Fighter& fighter)
{
  const auto& frame = fighter.frame();
  const auto& anchor = fighter.get_anchor();
  const Rect dest{ float(anchor.x + frame.drawOffset.x),
                   float(anchor.y + frame.drawOffset.y),
                   float(frame.spriteSourceW),
                   float(frame.spriteSourceH) };
  return FighterRender{ frame, dest };
}

} // namespace fight_scene_detail

FightScene
localFight(std::shared_ptr<FighterDefinition> player,
           std::shared_ptr<FighterDefinition> opponent)
{
  return FightScene(std::make_unique<KeyboardController>(
                      fight_scene_detail::buildPlayerKeymap()),
                    std::make_unique<KeyboardController>(
                      fight_scene_detail::buildSecondaryKeymap()),
                    player,
                    opponent);
}

void
FightScene::start()
{
  std::cout << "start todo\n";
}

void
FightScene::ingest(const std::vector<SDL_Event>& events)
{
  playerController->ingest(events, sim_tick);
  otherController->ingest(events, sim_tick);
}

void
FightScene::update()
{
  player.update(playerController->get_commands());
  other.update(otherController->get_commands());
  ++sim_tick;
}

void
FightScene::render(Graphics& graphics, AssetStore&)
{
  auto info = fight_scene_detail::texture_destination(player);
  graphics.render_texture(&info.frame, &info.destination);
  auto info2 = fight_scene_detail::texture_destination(other);
  graphics.render_texture(&info2.frame, &info2.destination);
}

FightScene::FightScene(std::unique_ptr<InputController> c1,
                       std::unique_ptr<InputController> c2,
                       std::shared_ptr<FighterDefinition> player_character,
                       std::shared_ptr<FighterDefinition> other_character)
  : player(0, Vec2{ 50, 100 }, player_character)
  , other(1, Vec2{ 100, 100 }, other_character)
  , playerController(std::move(c1))
  , otherController(std::move(c2))
{
}
