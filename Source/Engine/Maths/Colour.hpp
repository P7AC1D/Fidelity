#pragma once
#include "../Core/Types.hpp"
#include "Vector3.hpp"
#include "Vector4.hpp"

class Colour
{
public:
  static Colour Black;
  static Colour White;
  static Colour Red;
  static Colour Lime;  
  static Colour Blue;
  static Colour Yellow;
  static Colour Cyan;
  static Colour Magenta;
  static Colour Silver;
  static Colour Gray;
  static Colour Maroon;
  static Colour Olive;
  static Colour Green;
  static Colour Purple;
  static Colour Teal;
  static Colour Navy;

  Colour();
  explicit Colour(uint8 red, uint8 green = 255, uint8 blue = 255, uint8 alpha = 255);

  Vector3 ToVec3();
  Vector4 ToVec4();

private:
  bool _dirty;
  uint8 _red;
  uint8 _green;
  uint8 _blue;
  uint8 _alpha;
  Vector3 _rgbData;
  Vector4 _rgbaData;
};