#include "Colour.hpp"

#include <cassert>

Colour Colour::Black = Colour(0, 0, 0);
Colour Colour::White = Colour(255, 255, 255);
Colour Colour::Red = Colour(255, 0, 0);
Colour Colour::Lime = Colour(0, 255, 0);
Colour Colour::Blue = Colour(0, 0, 255);
Colour Colour::Yellow = Colour(255, 255, 0);
Colour Colour::Cyan = Colour(0, 255, 255);
Colour Colour::Magenta = Colour(255, 0, 255);
Colour Colour::Silver = Colour(192, 192, 192);
Colour Colour::Gray = Colour(128, 128, 128);
Colour Colour::Maroon = Colour(128, 0, 0);
Colour Colour::Olive = Colour(128, 128, 0);
Colour Colour::Green = Colour(0, 128, 0);
Colour Colour::Purple = Colour(128, 0, 128);
Colour Colour::Teal = Colour(0, 128, 128);
Colour Colour::Navy = Colour(0, 0, 128);

Colour::Colour() :
  _red(1.0f),
  _green(1.0f),
  _blue(1.0f),
  _alpha(1.0f)
{
}

Colour::Colour(uint8 red, uint8 green, uint8 blue, uint8 alpha)
{
  float32 f = 1 / 255.0f;
  _red = red * f;
  _green = green * f;
  _blue = blue * f;
  _alpha = alpha * f;
}

float32 Colour::operator[](int32 i) const
{
  assert(i < 4);
  return *(&_red + i);
}

