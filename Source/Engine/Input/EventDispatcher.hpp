#pragma once
#include <functional>
#include <unordered_map>
#include <vector>

#include "../Core/Types.hpp"

typedef std::string Action;
typedef std::string State;

class EventDispatcher
{
public:
  void RegisterActionDispatch(Action action, const std::function<void()>& actionCommand);
  void RegisterStateDispatch(State state, const std::function<void(uint32)>& stateCommand);

  void DispatchAction(Action action) const;
  void DispatchState(State state, uint32 dtMs) const;

private:
  std::unordered_map<Action, std::vector<std::function<void()>>> _actionCommandsMap;
  std::unordered_map<State, std::vector<std::function<void(uint32)>>> _stateCommandsMap;
};