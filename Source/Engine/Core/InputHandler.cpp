#include "InputHandler.h"

void InputHandler::setButtonState(Button button, ButtonState state)
{
  _buttonStates[button] = state;
}

void InputHandler::setAxisState(Axis axis, Vector2I axisState)
{
  _axesStates[axis] = axisState;
}

bool InputHandler::isButtonPressed(Button button) const
{
  auto findIter = _buttonStates.find(button);
  if (findIter == _buttonStates.end())
  {
    return false;
  }
  return findIter->second == ButtonState::Pressed;
}

bool InputHandler::isButtonReleased(Button button) const
{
  return !isButtonPressed(button);
}

Vector2I InputHandler::getAxisState(Axis axis) const
{
  auto findIter = _axesStates.find(axis);
  if (findIter == _axesStates.end())
  {
    return Vector2I();
  }
  return findIter->second;
}