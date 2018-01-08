#pragma once
#include <unordered_map>
#include <string>
#include <vector>

#include "../Core/Types.hpp"
#include "Buttons.hpp"
#include "Keys.hpp"

class EventDispatcher;
class Vector2;

typedef std::string Action;
typedef std::string State;

class InputHandler
{
public:
  InputHandler(const EventDispatcher& eventDispatcher);

  void BindKeyToAction(Action action, Key key);
  void BindButtonToAction(Action action, Button button);
  void BindKeyToState(State state, Key key);
  void BindButtonToState(State state, Button button);

  void SetKeyUp(Key key);
  void SetButtonUp(Button button);
  void SetKeyDown(Key key);
  void SetButtonDown(Button button);
  
  Vector2 GetMousePosition() const;

  void DispatchEvents(uint32 dtMs) const;

private:
  void DispatchKeyEvents(uint32 dtMs) const;
  void DispatchButtonEvents(uint32 dtMs) const;
  
  void DispatchKeyActions(const std::vector<Key>& keysPressed) const;
  void DispatchKeyStates(const std::vector<Key>& keysPressed, uint32 dt) const;
  void DispatchButtonActions(const std::vector<Button>& buttonsPressed) const;
  void DispatchButtonStates(const std::vector<Button>& buttonsPressed, uint32 dt) const;

private:
  std::unordered_map<Key, bool> _keysPressed;
  std::unordered_map<Button, bool> _buttonsPressed;
  std::unordered_map<Key, Action> _actionKeyBindings;
  std::unordered_map<Button, Action> _actionButtonBindings;
  std::unordered_map<Key, State> _stateKeyBindings;
  std::unordered_map<Button, State> _stateButtonBindings;

  const EventDispatcher& _eventDispatcher;
};
