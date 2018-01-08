#pragma once
#include <functional>
#include <unordered_map>
#include <string>
#include <vector>

#include "../Core/Types.hpp"
#include "InputHandler.hpp"

class EventDispatcher
{
public:
  void Register(Action action, const std::function<void(const InputEvent&)>& actionCommand);
  void Register(State state, const std::function<void(const InputEvent&, uint32)>& stateCommand);

  void Dispatch(State state, const InputEvent& inputEvent, uint32 dtMs) const;
  void Dispatch(Action action, const InputEvent& inputEvent) const;

private:
  std::unordered_map<Action, std::vector<std::function<void(const InputEvent&, uint32)>>> _stateCommandsMap;
  std::unordered_map<Action, std::vector<std::function<void(const InputEvent&)>>> _actionCommandsMap;
};
