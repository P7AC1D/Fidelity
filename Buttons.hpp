#pragma once
#include 

enum class Button
{
  MouseButton1,
  MouseButton2,
  MouseButton3,
}

Button ConvertSDLButton(int32 button);
