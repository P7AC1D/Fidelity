#pragma once
#include "../Core/Types.hpp"

enum class Button
{
  MouseButtonLeft,
  MouseButtonRight,
  MouseButtonMiddle
};

Button ConvertSDLButton(int8 button);
