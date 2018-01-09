#include "InputHandler.hpp"

#include "EventDispatcher.hpp"

InputHandler::InputHandler(const EventDispatcher& eventDispatcher)
: _eventDispatcher(eventDispatcher)
{
  for (auto& button : _buttonsDown)
  {
    button = false;
  }
}

void InputHandler::BindButtonToAction(const Action& action, Button button)
{
  _buttonActionBindings[button] = action;
}

void InputHandler::BindButtonToState(const State& state, Button button)
{
  _buttonStateBindings[button] = state;
}

void InputHandler::BindAxisToState(const State& state, Axis axis)
{
  _axisStateBindings[axis] = state;
}

void InputHandler::Dispatch(const InputEvent& inputEvent, uint32 dtMs)
{
  if (inputEvent.Button != Button::_null)
  {
    DispatchButtonActions(inputEvent);
    DispatchButtonStates(inputEvent, dtMs);
  }

  if (inputEvent.Axis != Axis::_null)
  {
    DispatchAxisStates(inputEvent, dtMs);
  }
}

bool InputHandler::IsButtonStateActive(const State& state) const
{
  auto iter = std::find_if(_buttonStateBindings.begin(), _buttonStateBindings.end(), [&](const std::pair<Button, State>& value)
  {
    return value.second == state;
  });
  if (iter != _buttonStateBindings.end())
  {
    return _buttonsDown[static_cast<size_t>(iter->first)];
  }
  return false;
}

void InputHandler::DispatchButtonActions(const InputEvent& inputEvent)
{
  if (inputEvent.ButtonEvent == ButtonEvent::Pressed)
  {
    auto actionIter = _buttonActionBindings.find(inputEvent.Button);
    if (actionIter != _buttonActionBindings.end())
    {
      _eventDispatcher.Dispatch(actionIter->second, inputEvent);
    }

    _buttonsDown[static_cast<size_t>(inputEvent.Button)] = true;
  }
  else if (inputEvent.ButtonEvent == ButtonEvent::Released)
  {
    _buttonsDown[static_cast<size_t>(inputEvent.Button)] = false;
  }
}

void InputHandler::DispatchButtonStates(const InputEvent& inputEvent, uint32 dtMs)
{
  if (inputEvent.ButtonEvent == ButtonEvent::Pressed)
  {
    _buttonsDown[static_cast<size_t>(inputEvent.Button)] = true;
  }
  else if (inputEvent.ButtonEvent == ButtonEvent::Released)
  {
    _buttonsDown[static_cast<size_t>(inputEvent.Button)] = false;
  }

  if (_buttonsDown[static_cast<size_t>(inputEvent.Button)])
  {
    auto stateIter = _buttonStateBindings.find(inputEvent.Button);
    if (stateIter != _buttonStateBindings.end())
    {
      _eventDispatcher.Dispatch(stateIter->second, inputEvent, dtMs);
    }
  }
}

void InputHandler::DispatchAxisStates(const InputEvent& inputEvent, uint32 dtMs)
{
  auto axisIter = _axisStateBindings.find(inputEvent.Axis);
  if (axisIter != _axisStateBindings.end())
  {
    _eventDispatcher.Dispatch(axisIter->second, inputEvent, dtMs);
  }
}