#include "InputHandler.hpp"

#include "EventDispatcher.hpp"

InputHandler::InputHandler(const EventDispatcher& eventDispatcher)
: _eventDispatcher(eventDispatcher)
{
}

void InputHandler::BindKeyToAction(Action action, Key key)
{
  _actionKeyBindings[key] = action;
}

void InputHandler::BindKeyToState(State state, Key key)
{
  _stateKeyBindings[key] = state;
}

void InputHandler::SetKeyUp(Key key)
{
  _keysPressed[key] = false;
}

void InputHandler::SetKeyDown(Key key)
{
  _keysPressed[key] = true;
}

void InputHandler::DispatchEvents(uint32 dtMs) const
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

  DispatchActions(keysPressed);
  DispatchStates(keysPressed, dtMs);
}

void InputHandler::DispatchActions(const std::vector<Key>& keysPressed) const
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

void InputHandler::DispatchStates(const std::vector<Key>& keysPressed, uint32 dtMs) const
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