#include "EventDispatcher.hpp"

void EventDispatcher::Register(const Action& action, const std::function<void(const InputEvent& inputEvent)>& actionCommand)
{
  _actionCommandsMap[action].push_back(actionCommand);
}

void EventDispatcher::Register(const State& state, const std::function<void(const InputEvent& inputEvent, int32)>& stateCommand)
{
  _stateCommandsMap[state].push_back(stateCommand);
}

void EventDispatcher::Dispatch(const State& state, const InputEvent& inputEvent, int32 dtMs) const
{
  auto iter = _stateCommandsMap.find(state);
  if (iter != _stateCommandsMap.end())
  {
    for (auto& command : iter->second)
    {
      command(inputEvent, dtMs);
    }
  }
}

void EventDispatcher::Dispatch(const Action& action, const InputEvent& inputEvent) const
{
  auto iter = _actionCommandsMap.find(action);
  if (iter != _actionCommandsMap.end())
  {
    for (auto& command : iter->second)
    {
      command(inputEvent);
    }
  }
}