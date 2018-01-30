#include "InputBindings.hpp"

#include <stdexcept>

#include <SDL.h>

Button SDLToButton(int32 button)
{
  switch (button)
  {
    case SDLK_ESCAPE: return Button::Key_Escape;
    case SDLK_F1: return Button::Key_F1;
    case SDLK_F2: return Button::Key_F2;
    case SDLK_F3: return Button::Key_F3;
    case SDLK_F4: return Button::Key_F4;
    case SDLK_F5: return Button::Key_F5;
    case SDLK_F6: return Button::Key_F6;
    case SDLK_F7: return Button::Key_F7;
    case SDLK_F8: return Button::Key_F8;
    case SDLK_F9: return Button::Key_F9;
    case SDLK_F10: return Button::Key_F10;
    case SDLK_F11: return Button::Key_F11;
    case SDLK_F12: return Button::Key_F12;
    case SDLK_BACKQUOTE: return Button::Key_BackQuote;
    case SDLK_1: return Button::Key_1;
    case SDLK_2: return Button::Key_2;
    case SDLK_3: return Button::Key_3;
    case SDLK_4: return Button::Key_4;
    case SDLK_5: return Button::Key_5;
    case SDLK_6: return Button::Key_6;
    case SDLK_7: return Button::Key_7;
    case SDLK_8: return Button::Key_8;
    case SDLK_9: return Button::Key_9;
    case SDLK_0: return Button::Key_0;
    case SDLK_MINUS: return Button::Key_Minus;
    case SDLK_EQUALS: return Button::Key_Equals;
    case SDLK_BACKSPACE: return Button::Key_BackSpace;
    case SDLK_TAB: return Button::Key_Tab;
    case SDLK_q: return Button::Key_Q;
    case SDLK_w: return Button::Key_W;
    case SDLK_e: return Button::Key_E;
    case SDLK_r: return Button::Key_R;
    case SDLK_t: return Button::Key_T;
    case SDLK_y: return Button::Key_Y;
    case SDLK_u: return Button::Key_U;
    case SDLK_i: return Button::Key_I;
    case SDLK_o: return Button::Key_O;
    case SDLK_p: return Button::Key_P;
    case SDLK_LEFTBRACKET: return Button::Key_LBracket;
    case SDLK_RIGHTBRACKET: return Button::Key_RBracket;
    case SDLK_BACKSLASH: return Button::Key_BackSlash;
    case SDLK_CAPSLOCK: return Button::Key_CapsLock;
    case SDLK_a: return Button::Key_A;
    case SDLK_s: return Button::Key_S;
    case SDLK_d: return Button::Key_D;
    case SDLK_f: return Button::Key_F;
    case SDLK_g: return Button::Key_G;
    case SDLK_h: return Button::Key_H;
    case SDLK_j: return Button::Key_J;
    case SDLK_k: return Button::Key_K;
    case SDLK_l: return Button::Key_L;
    case SDLK_SEMICOLON: return Button::Key_SemiColon;
    case SDLK_ALTERASE: return Button::Key_Apostrophe;
    case SDLK_RETURN: return Button::Key_Return;
    case SDLK_LSHIFT: return Button::Key_LShift;
    case SDLK_z: return Button::Key_Z;
    case SDLK_x: return Button::Key_X;
    case SDLK_c: return Button::Key_C;
    case SDLK_v: return Button::Key_V;
    case SDLK_b: return Button::Key_B;
    case SDLK_n: return Button::Key_N;
    case SDLK_m: return Button::Key_M;
    case SDLK_COMMA: return Button::Key_Comma;
    case SDLK_PERIOD: return Button::Key_Period;
    case SDLK_SLASH: return Button::Key_ForwardSlash;
    case SDLK_RSHIFT: return Button::Key_RShift;
    case SDLK_LCTRL: return Button::Key_LCtrl;
    case SDLK_LALT: return Button::Key_LAlt;
    case SDLK_SPACE: return Button::Key_Space;
    case SDLK_RALT: return Button::Key_RAlt;
    case SDLK_RCTRL: return Button::Key_RCtrl;
    case SDLK_INSERT: return Button::Key_Insert;
    case SDLK_HOME: return Button::Key_Home;
    case SDLK_PAGEUP: return Button::Key_PageUp;
    case SDLK_DELETE: return Button::Key_Delete;
    case SDLK_END: return Button::Key_End;
    case SDLK_PAGEDOWN: return Button::Key_PageDown;
    case SDLK_UP: return Button::Key_Up;
    case SDLK_LEFT: return Button::Key_Left;
    case SDLK_DOWN: return Button::Key_Down;
    case SDLK_RIGHT: return Button::Key_Right;
    case SDLK_KP_DIVIDE: return Button::Key_NumDivide;
    case SDLK_KP_MULTIPLY: return Button::Key_NumMultiply;
    case SDLK_KP_MINUS: return Button::Key_NumMinus;
    case SDLK_KP_1: return Button::Key_Num1;
    case SDLK_KP_2: return Button::Key_Num2;
    case SDLK_KP_3: return Button::Key_Num3;
    case SDLK_KP_4: return Button::Key_Num4;
    case SDLK_KP_5: return Button::Key_Num5;
    case SDLK_KP_6: return Button::Key_Num6;
    case SDLK_KP_7: return Button::Key_Num7;
    case SDLK_KP_8: return Button::Key_Num8;
    case SDLK_KP_9: return Button::Key_Num9;
    case SDLK_KP_0: return Button::Key_Num0;
    case SDLK_KP_PLUS: return Button::Key_NumPlus;
    case SDLK_KP_ENTER: return Button::Key_NumReturn;
    case SDLK_KP_PERIOD: return Button::Key_NumPeriod;
    case SDL_BUTTON_LEFT: return Button::Button_LMouse;
    case SDL_BUTTON_RIGHT: return Button::Button_RMouse;
    case SDL_BUTTON_MIDDLE: return Button::Button_Mouse3;
    case SDL_BUTTON_X1: return Button::Button_Mouse4;
    case SDL_BUTTON_X2: return Button::Button_Mouse5;
    default: throw std::runtime_error("Unsupported SDL button");
  }
}
