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

  Vector3 ToVec3() { return Vector3(_red, _green, _blue); }
  Vector4 ToVec4() { return Vector4(_red, _green, _blue, _alpha); }
  
  float32 operator[](int32 i) const;

private:
  float32 _red;
  float32 _green;
  float32 _blue;
  float32 _alpha;
};
