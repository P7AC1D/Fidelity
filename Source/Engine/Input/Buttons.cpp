#include "Buttons.hpp"

#include <SDL.h>

Button ConvertSDLButton(int8 button)
{
  switch (button)
  {
    case SDL_BUTTON_LEFT: return Button::MouseButtonLeft;
    case SDL_BUTTON_RIGHT: return Button::MouseButtonRight;
    case SDL_BUTTON_MIDDLE: return Button::MouseButtonMiddle;
  }
}
