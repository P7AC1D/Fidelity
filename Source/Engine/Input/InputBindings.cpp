#include "InputBindings.hpp"

#include <stdexcept>

#include <GLFW/glfw3.h>

Button GlfwKeyToButton(int32 button)
{
  switch (button)
  {
  case GLFW_KEY_ESCAPE: return Button::Key_Escape;
  case GLFW_KEY_F1: return Button::Key_F1;
  case GLFW_KEY_F2: return Button::Key_F2;
  case GLFW_KEY_F3: return Button::Key_F3;
  case GLFW_KEY_F4: return Button::Key_F4;
  case GLFW_KEY_F5: return Button::Key_F5;
  case GLFW_KEY_F6: return Button::Key_F6;
  case GLFW_KEY_F7: return Button::Key_F7;
  case GLFW_KEY_F8: return Button::Key_F8;
  case GLFW_KEY_F9: return Button::Key_F9;
  case GLFW_KEY_F10: return Button::Key_F10;
  case GLFW_KEY_F11: return Button::Key_F11;
  case GLFW_KEY_F12: return Button::Key_F12;
  case GLFW_KEY_GRAVE_ACCENT: return Button::Key_BackQuote;
  case GLFW_KEY_1: return Button::Key_1;
  case GLFW_KEY_2: return Button::Key_2;
  case GLFW_KEY_3: return Button::Key_3;
  case GLFW_KEY_4: return Button::Key_4;
  case GLFW_KEY_5: return Button::Key_5;
  case GLFW_KEY_6: return Button::Key_6;
  case GLFW_KEY_7: return Button::Key_7;
  case GLFW_KEY_8: return Button::Key_8;
  case GLFW_KEY_9: return Button::Key_9;
  case GLFW_KEY_0: return Button::Key_0;
  case GLFW_KEY_MINUS: return Button::Key_Minus;
  case GLFW_KEY_EQUAL: return Button::Key_Equals;
  case GLFW_KEY_BACKSPACE: return Button::Key_BackSpace;
  case GLFW_KEY_TAB: return Button::Key_Tab;
  case GLFW_KEY_Q: return Button::Key_Q;
  case GLFW_KEY_W: return Button::Key_W;
  case GLFW_KEY_E: return Button::Key_E;
  case GLFW_KEY_R: return Button::Key_R;
  case GLFW_KEY_T: return Button::Key_T;
  case GLFW_KEY_Y: return Button::Key_Y;
  case GLFW_KEY_U: return Button::Key_U;
  case GLFW_KEY_I: return Button::Key_I;
  case GLFW_KEY_O: return Button::Key_O;
  case GLFW_KEY_P: return Button::Key_P;
  case GLFW_KEY_LEFT_BRACKET: return Button::Key_LBracket;
  case GLFW_KEY_RIGHT_BRACKET: return Button::Key_RBracket;
  case GLFW_KEY_BACKSLASH: return Button::Key_BackSlash;
  case GLFW_KEY_CAPS_LOCK: return Button::Key_CapsLock;
  case GLFW_KEY_A: return Button::Key_A;
  case GLFW_KEY_S: return Button::Key_S;
  case GLFW_KEY_D: return Button::Key_D;
  case GLFW_KEY_F: return Button::Key_F;
  case GLFW_KEY_G: return Button::Key_G;
  case GLFW_KEY_H: return Button::Key_H;
  case GLFW_KEY_J: return Button::Key_J;
  case GLFW_KEY_K: return Button::Key_K;
  case GLFW_KEY_L: return Button::Key_L;
  case GLFW_KEY_SEMICOLON: return Button::Key_SemiColon;
  case GLFW_KEY_APOSTROPHE: return Button::Key_Apostrophe;
  case GLFW_KEY_ENTER: return Button::Key_Return;
  case GLFW_KEY_LEFT_SHIFT: return Button::Key_LShift;
  case GLFW_KEY_Z: return Button::Key_Z;
  case GLFW_KEY_X: return Button::Key_X;
  case GLFW_KEY_C: return Button::Key_C;
  case GLFW_KEY_V: return Button::Key_V;
  case GLFW_KEY_B: return Button::Key_B;
  case GLFW_KEY_N: return Button::Key_N;
  case GLFW_KEY_M: return Button::Key_M;
  case GLFW_KEY_COMMA: return Button::Key_Comma;
  case GLFW_KEY_PERIOD: return Button::Key_Period;
  case GLFW_KEY_SLASH: return Button::Key_ForwardSlash;
  case GLFW_KEY_RIGHT_SHIFT: return Button::Key_RShift;
  case GLFW_KEY_LEFT_CONTROL: return Button::Key_LCtrl;
  case GLFW_KEY_LEFT_ALT: return Button::Key_LAlt;
  case GLFW_KEY_SPACE: return Button::Key_Space;
  case GLFW_KEY_RIGHT_ALT: return Button::Key_RAlt;
  case GLFW_KEY_RIGHT_CONTROL: return Button::Key_RCtrl;
  case GLFW_KEY_INSERT: return Button::Key_Insert;
  case GLFW_KEY_HOME: return Button::Key_Home;
  case GLFW_KEY_PAGE_UP: return Button::Key_PageUp;
  case GLFW_KEY_DELETE: return Button::Key_Delete;
  case GLFW_KEY_END: return Button::Key_End;
  case GLFW_KEY_PAGE_DOWN: return Button::Key_PageDown;
  case GLFW_KEY_UP: return Button::Key_Up;
  case GLFW_KEY_LEFT: return Button::Key_Left;
  case GLFW_KEY_DOWN: return Button::Key_Down;
  case GLFW_KEY_RIGHT: return Button::Key_Right;
  case GLFW_KEY_KP_DIVIDE: return Button::Key_NumDivide;
  case GLFW_KEY_KP_MULTIPLY: return Button::Key_NumMultiply;
  case GLFW_KEY_KP_SUBTRACT: return Button::Key_NumMinus;
  case GLFW_KEY_KP_1: return Button::Key_Num1;
  case GLFW_KEY_KP_2: return Button::Key_Num2;
  case GLFW_KEY_KP_3: return Button::Key_Num3;
  case GLFW_KEY_KP_4: return Button::Key_Num4;
  case GLFW_KEY_KP_5: return Button::Key_Num5;
  case GLFW_KEY_KP_6: return Button::Key_Num6;
  case GLFW_KEY_KP_7: return Button::Key_Num7;
  case GLFW_KEY_KP_8: return Button::Key_Num8;
  case GLFW_KEY_KP_9: return Button::Key_Num9;
  case GLFW_KEY_KP_0: return Button::Key_Num0;
  case GLFW_KEY_KP_ADD: return Button::Key_NumPlus;
  case GLFW_KEY_KP_ENTER: return Button::Key_NumReturn;
    //case GLFW_KEY_KP_PERIOD: return Button::Key_NumPeriod;
  default: throw std::runtime_error("Unsupported GLFW button");
  }
}

  Button GlfwMouseButtonToButton(int32 button)
  {
    switch (button)
    {
    case GLFW_MOUSE_BUTTON_1: return Button::Button_LMouse;
    case GLFW_MOUSE_BUTTON_2: return Button::Button_RMouse;
    case GLFW_MOUSE_BUTTON_3: return Button::Button_Mouse3;
    case GLFW_MOUSE_BUTTON_4: return Button::Button_Mouse4;
    case GLFW_MOUSE_BUTTON_5: return Button::Button_Mouse5;
    default: throw std::runtime_error("Unsupported GLFW button");
    }
  }
