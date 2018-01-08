#include "InputHandler.hpp"

#include <SDL.h>

#include "../Maths/Vector2.hpp"
#include "EventDispatcher.hpp"

InputHandler::InputHandler(const EventDispatcher& eventDispatcher)
: _eventDispatcher(eventDispatcher)
{
}

void InputHandler::BindKeyToAction(Action action, Key key)
{
  _actionKeyBindings[key] = action;
}

void InputHandler::BindButtonToAction(Action action, Button button)
{
  _stateButtonBindings[button] = action;
}

void InputHandler::BindKeyToState(State state, Key key)
{
  _stateKeyBindings[key] = state;
}

void InputHandler::BindButtonToState(State state, Button button)
{
  _stateButtonBindings[button] = state;
}

void InputHandler::SetKeyUp(Key key)
{
  _keysPressed[key] = false;
}

void InputHandler::SetButtonUp(Button button)
{
  _buttonsPressed[button] = false;
}

void InputHandler::SetKeyDown(Key key)
{
  _keysPressed[key] = true;
}

void InputHandler::SetButtonDown(Button button)
{
  _buttonsPressed[button] = true;
}

Vector2 InputHandler::GetMousePosition() const
{
  int32 xPos = 0;
  int32 yPos = 0;
  SDL_GetMouseState(&xPos, &yPos);
  return Vector2(xPos, yPos);
}

void InputHandler::DispatchEvents(uint32 dtMs) const
{
  DispatchKeyEvents(dtMs);
  DispatchButtonEvents(dtMs);
}

void InputHandler::DispatchKeyEvents(uint32 dtMs) const
{
  std::vector<Key> keysPressed;
  keysPressed.reserve(_keysPressed.size());
  for (auto& key : _keysPressed)
  {
    if (key.second)
    {
      keysPressed.push_back(key.first);
    }
  }
  DispatchKeyActions(keysPressed);
  DispatchKeyStates(keysPressed, dtMs);
}

void InputHandler::DispatchButtonEvents(uint32 dtMs) const
{
  std::vector<Button> buttonsPressed;
  buttonsPressed.reserve(_buttonsPressed.size());
  for (auto& button : _buttonsPressed)
  {
    if (button.second)
    {
      buttonsPressed.push_back(button.first);
    }
  }
  DispatchButtonActions(buttonsPressed);
  DispatchButtonStates(buttonsPressed, dtMs);
}

void InputHandler::DispatchKeyActions(const std::vector<Key>& keysPressed) const
{
  for (auto& key : keysPressed)
  {
    auto iter = _actionKeyBindings.find(key);
    if (iter != _actionKeyBindings.end())
    {
      _eventDispatcher.DispatchAction(iter->second);
    }
  }
}

void InputHandler::DispatchKeyStates(const std::vector<Key>& keysPressed, uint32 dtMs) const
{
  for (auto& key : keysPressed)
  {
    auto iter = _stateKeyBindings.find(key);
    if (iter != _stateKeyBindings.end())
    {
      _eventDispatcher.DispatchState(iter->second, dtMs);
    }
  }
}

void InputHandler::DispatchButtonActions(const std::vector<Button>& buttonsPressed) const
{
  for (auto& key : buttonsPressed)
  {
    auto iter = _actionButtonBindings.find(key);
    if (iter != _actionButtonBindings.end())
    {
      _eventDispatcher.DispatchAction(iter->second);
    }
  }
}

void InputHandler::DispatchButtonStates(const std::vector<Button>& buttonsPressed, uint32 dtMs) const
{
  for (auto& key : buttonsPressed)
  {
    auto iter = _stateButtonBindings.find(key);
    if (iter != _stateButtonBindings.end())
    {
      _eventDispatcher.DispatchState(iter->second, dtMs);
    }
  }
}
