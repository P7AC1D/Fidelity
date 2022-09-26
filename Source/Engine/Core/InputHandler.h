#pragma once
#include <unordered_map>

#include "../Maths/Vector2I.hpp"
#include "Types.hpp"

enum class Button
{
  Key_Escape,
  Key_F1,
  Key_F2,
  Key_F3,
  Key_F4,
  Key_F5,
  Key_F6,
  Key_F7,
  Key_F8,
  Key_F9,
  Key_F10,
  Key_F11,
  Key_F12,
  Key_BackQuote,
  Key_1,
  Key_2,
  Key_3,
  Key_4,
  Key_5,
  Key_6,
  Key_7,
  Key_8,
  Key_9,
  Key_0,
  Key_Minus,
  Key_Equals,
  Key_BackSpace,
  Key_Tab,
  Key_Q,
  Key_W,
  Key_E,
  Key_R,
  Key_T,
  Key_Y,
  Key_U,
  Key_I,
  Key_O,
  Key_P,
  Key_LBracket,
  Key_RBracket,
  Key_BackSlash,
  Key_CapsLock,
  Key_A,
  Key_S,
  Key_D,
  Key_F,
  Key_G,
  Key_H,
  Key_J,
  Key_K,
  Key_L,
  Key_SemiColon,
  Key_Apostrophe,
  Key_Return,
  Key_LShift,
  Key_Z,
  Key_X,
  Key_C,
  Key_V,
  Key_B,
  Key_N,
  Key_M,
  Key_Comma,
  Key_Period,
  Key_ForwardSlash,
  Key_RShift,
  Key_LCtrl,
  Key_LAlt,
  Key_Space,
  Key_RAlt,
  Key_RCtrl,
  Key_Insert,
  Key_Home,
  Key_PageUp,
  Key_Delete,
  Key_End,
  Key_PageDown,
  Key_Left,
  Key_Up,
  Key_Right,
  Key_Down,
  Key_NumDivide,
  Key_NumMultiply,
  Key_NumMinus,
  Key_Num1,
  Key_Num2,
  Key_Num3,
  Key_Num4,
  Key_Num5,
  Key_Num6,
  Key_Num7,
  Key_Num8,
  Key_Num9,
  Key_Num0,
  Key_NumPeriod,
  Key_NumPlus,
  Key_NumReturn,

  Button_LMouse,
  Button_RMouse,
  Button_Mouse3,
  Button_Mouse4,
  Button_Mouse5
};

enum class ButtonState
{
  Pressed,
  Released
};

enum class Axis
{
  MouseXY,
  MouseScrollXY
};

Button GlfwKeyToButton(int32 button);
Button GlfwMouseButtonToButton(int32 button);

class InputHandler
{
public:
  void setButtonState(Button button, ButtonState state);
  void setAxisState(Axis axis, Vector2I axisState);

  bool isButtonPressed(Button button) const;
  bool isButtonReleased(Button button) const;
  Vector2I getAxisState(Axis axis) const;

private:
  std::unordered_map<Button, ButtonState> _buttonStates;
  std::unordered_map<Axis, Vector2I> _axesStates;
};