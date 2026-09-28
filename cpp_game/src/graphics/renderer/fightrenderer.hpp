#pragma once

#include "../../combat/scene.hpp"
#include "../graphics.hpp"
#include "../texturestore.hpp"

class FightRenderer
{
private:
  TextureStore textures;

public:
  void render(const FightScene&, Graphics&);
}
