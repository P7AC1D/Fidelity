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
  void Register(const Action& action, const std::function<void(const InputEvent&)>& actionCommand);
  void Register(const State& state, const std::function<void(const InputEvent&, int32)>& stateCommand);

  void Dispatch(const State& state, const InputEvent& inputEvent, int32 dtMs) const;
  void Dispatch(const Action& action, const InputEvent& inputEvent) const;

private:
  std::unordered_map<Action, std::vector<std::function<void(const InputEvent&, int32)>>> _stateCommandsMap;
  std::unordered_map<Action, std::vector<std::function<void(const InputEvent&)>>> _actionCommandsMap;
};
