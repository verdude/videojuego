#include "scene.hpp"

#include "../geometry/types.hpp"
#include "../input/input.hpp"
#include "../input/keyboardcontroller.hpp"
#include "fighter.hpp"

#include <SDL3/SDL_scancode.h>
#include <iostream>
#include <map>

namespace fight_scene_detail {

std::map<Control, SDL_Scancode>
buildPlayerKeymap()
{
  std::map<Control, SDL_Scancode> playerKeymap{};
  playerKeymap[Control::Left] = SDL_SCANCODE_A;
  playerKeymap[Control::Right] = SDL_SCANCODE_D;
  playerKeymap[Control::One] = SDL_SCANCODE_J;
  return playerKeymap;
}

std::map<Control, SDL_Scancode>
buildSecondaryKeymap()
{
  std::map<Control, SDL_Scancode> otherKeymap{};
  otherKeymap[Control::Left] = SDL_SCANCODE_LEFT;
  otherKeymap[Control::Right] = SDL_SCANCODE_RIGHT;
  otherKeymap[Control::One] = SDL_SCANCODE_KP_0;
  return otherKeymap;
}

const FighterRender
texture_destination(Fighter& fighter)
{
  const auto& frame = fighter.frame();
  const auto& anchor = fighter.get_anchor();
  const bool flip = fighter.get_facing() != Facing::Right;

  const float width = float(frame.spriteSourceW);
  const float offsetX = float(frame.drawOffset.x);

  const Rect dest{
    float(anchor.x) + (flip ? -offsetX - width : offsetX),
    float(anchor.y) + float(frame.drawOffset.y),
    width,
    float(frame.spriteSourceH)
  };

  return FighterRender{ frame, dest, flip };
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
  player_controller->ingest(events, sim_tick);
  other_controller->ingest(events, sim_tick);
}

void
FightScene::update()
{
  player.advance_move_time();
  other.advance_move_time();

  bool face_right = player.get_anchor().left_of(other.get_anchor());
  player.update_facing(face_right);
  other.update_facing(!face_right);

  player.update(player_controller->get_commands(),
                player_controller->get_held());
  other.update(other_controller->get_commands(), other_controller->get_held());

  ++sim_tick;
}

void
FightScene::render(Graphics& graphics, AssetStore&)
{
  auto info = fight_scene_detail::texture_destination(player);
  graphics.render_texture(&info.frame, &info.destination, info.flip);
  auto info2 = fight_scene_detail::texture_destination(other);
  graphics.render_texture(&info2.frame, &info2.destination, info2.flip);
}

FightScene::FightScene(std::unique_ptr<InputController> c1,
                       std::unique_ptr<InputController> c2,
                       std::shared_ptr<FighterDefinition> player_character,
                       std::shared_ptr<FighterDefinition> other_character)
  : player(0, Vec2{ 50, 100 }, Facing::Right, player_character)
  , other(1, Vec2{ 100, 100 }, Facing::Left, other_character)
  , player_controller(std::move(c1))
  , other_controller(std::move(c2))
  , cmd_recognizer("data/commands.json")
{
  cmd_recognizer.load();
}
