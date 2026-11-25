#pragma once

// Same as SDL_FRect for rendering
struct Rect
{
  float x, y, w, h;
};

struct Vec2
{
  int x, y;

  bool left_of(const Vec2& other) const { return x <= other.x; }
};
