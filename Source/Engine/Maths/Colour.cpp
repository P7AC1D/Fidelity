#include "Colour.hpp"

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
  _dirty(true),
  _red(255),
  _green(255),
  _blue(255),
  _alpha(255)
{
}

Colour::Colour(uint8 red, uint8 green, uint8 blue, uint8 alpha) :
  _dirty(true),
  _red(red),
  _green(green),
  _blue(blue),
  _alpha(alpha)
{
}

Vector3 Colour::ToVec3()
{
  if (_dirty)
  {
    float32 f = 1 / 255.0f;
    _rgbData = Vector3(_red * f, _green * f, _blue * f);
    _dirty = false;
  }
  return _rgbData;
}
Vector4 Colour::ToVec4()
{
  if (_dirty)
  {
    float32 f = 1 / 255.0f;
    _rgbaData = Vector4(_red * f, _green * f, _blue * f, _alpha * f);
    _dirty = false;
  }
  return _rgbaData;
}