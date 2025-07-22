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

bool InputHandler::wasButtonJustPressed(Button button) const
{
  // Check if button is currently pressed but was not pressed in the previous frame
  bool currentlyPressed = isButtonPressed(button);
  bool previouslyPressed = false;

  auto prevIter = _previousButtonStates.find(button);
  if (prevIter != _previousButtonStates.end())
  {
    previouslyPressed = (prevIter->second == ButtonState::Pressed);
  }

  return currentlyPressed && !previouslyPressed;
}

void InputHandler::updatePreviousStates()
{
  _previousButtonStates = _buttonStates;
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