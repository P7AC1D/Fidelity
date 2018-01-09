#pragma once
#include <array>
#include <unordered_map>
#include <string>

#include "../Core/Types.hpp"
#include "../Maths/Vector2i.hpp"
#include "InputBindings.hpp"

class EventDispatcher;

typedef std::string Action;
typedef std::string State;

enum class ButtonEvent
{
  _null,

  Pressed,
  Released,
};

struct InputEvent
{
  Button Button = Button::_null;
  ButtonEvent ButtonEvent = ButtonEvent::_null;
  Axis Axis = Axis::_null;
  Vector2i AxisPos = Vector2i::Zero;
  Vector2i AxisPosDelta = Vector2i::Zero;
};

class InputHandler
{
public:
  InputHandler(const EventDispatcher& eventDispatcher);

  void BindButtonToAction(const Action& action, Button button);
  void BindButtonToState(const State& state, Button button);
  void BindAxisToState(const State& state, Axis axis);

  void Dispatch(const InputEvent& inputEvent, uint32 dtMs);

  bool IsButtonStateActive(const State& state) const;

private:
  void DispatchButtonActions(const InputEvent& inputEvent);
  void DispatchButtonStates(const InputEvent& inputEvent, uint32 dtMs);
  void DispatchAxisStates(const InputEvent& inputEvent, uint32 dtMs);

private:
  std::unordered_map<Button, Action> _buttonActionBindings;
  std::unordered_map<Button, State> _buttonStateBindings;
  std::unordered_map<Axis, State> _axisStateBindings;

  std::array<bool, static_cast<size_t>(Button::_count)> _buttonsDown;
  std::array<float32, static_cast<size_t>(Axis::_count)> _currentAxisValues;
  std::array<float32, static_cast<size_t>(Axis::_count)> _prevAxisValues;

  const EventDispatcher& _eventDispatcher;
};
