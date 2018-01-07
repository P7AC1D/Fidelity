#pragma once
#include <unordered_map>
#include <string>
#include <vector>

#include "../Core/Types.hpp"
#include "Keys.hpp"

class EventDispatcher;

typedef std::string Action;
typedef std::string State;

class InputHandler
{
public:
  InputHandler(const EventDispatcher& eventDispatcher);

  void BindKeyToAction(Action action, Key key);
  void BindKeyToState(State state, Key key);

  void SetKeyUp(Key key);
  void SetKeyDown(Key key);

  void DispatchEvents(uint32 dtMs) const;

private:
  void DispatchActions(const std::vector<Key>& keysPressed) const;
  void DispatchStates(const std::vector<Key>& keysPressed, uint32 dt) const;

private:
  std::unordered_map<Key, bool> _keysPressed;
  std::unordered_map<Key, Action> _actionKeyBindings;
  std::unordered_map<Key, State> _stateKeyBindings;

  const EventDispatcher& _eventDispatcher;
};