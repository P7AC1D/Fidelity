#include "EventDispatcher.hpp"

void EventDispatcher::RegisterActionDispatch(Action action, const std::function<void()>& actionCommand)
{
  _actionCommandsMap[action].push_back(actionCommand);
}

void EventDispatcher::RegisterStateDispatch(State state, const std::function<void(uint32 dt)>& stateCommand)
{
  _stateCommandsMap[state].push_back(stateCommand);
}

void EventDispatcher::DispatchAction(Action action) const
{
  auto iter = _actionCommandsMap.find(action);
  if (iter == _actionCommandsMap.end())
  {
    return;
  }

  for (auto& command : iter->second)
  {
    command();
  }
}

void EventDispatcher::DispatchState(State state, uint32 dtMs) const
{
  auto iter = _stateCommandsMap.find(state);
  if (iter == _stateCommandsMap.end())
  {
    return;
  }

  for (auto& command : iter->second)
  {
    command(dtMs);
  }
}