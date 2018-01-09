#pragma once
#include "../Core/Types.hpp"

enum class Key
{
  Q,
  W,
  E,
  R,
  T,
  Y,
  U,
  I,
  O,
  P,
  LeftBracket,
  RightBracket,
  BackSlash,
  A,
  S,
  D,
  F,
  G,
  H,
  J,
  K,
  L,
  SemiColon,
  Apostrophe,
  Z,
  X,
  C,
  V,
  B,
  N,
  M,
  Comma,
  FullStop,
  ForwardSlash
};

Key ConvertSDLKeyCode(int32 sdlKeyCode);