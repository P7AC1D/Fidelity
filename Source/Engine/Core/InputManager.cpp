#include "InputManager.h"

namespace Platform
{
InputManager::InputManager() :
  _mouseScrollOffset(0.0f)
{
}

InputManager::~InputManager()
{
}

void InputManager::SetMousePositionCallback(const std::function<void(float64& x, float64& y)>& mousePositionCallback)
{
  _mousePositionCallback = mousePositionCallback;
}

void InputManager::SetKeyDownEvent(int32 key, const std::function<void()>& onKeyDown)
{
  auto iter = _keyEvents.find(key);
  if (iter == _keyEvents.end())
  {
    auto events = std::vector<std::function<void()>>();
    events.push_back(onKeyDown);
    _keyEvents[key] = events;
    return;
  }
  iter->second.push_back(onKeyDown);
}

void InputManager::SetKeyDown(int32 key)
{
  auto iter = _keysPressed.find(key);
  if (iter == _keysPressed.end())
  {
    _keysPressed.insert(std::make_pair(key, true));
    return;
  }
  iter->second = true;
}

void InputManager::SetKeyUp(int32 key)
{
  auto iter = _keysPressed.find(key);
  if (iter == _keysPressed.end())
  {
    _keysPressed.insert(std::make_pair(key, false));
    return;
  }
  iter->second = false;
}

void InputManager::SetMouseButtonDown(int32 button)
{
  auto iter = _buttonsPressed.find(button);
  if (iter == _buttonsPressed.end())
  {
    _buttonsPressed.insert(std::make_pair(button, true));
    return;
  }
  iter->second = true;
}

void InputManager::SetMouseButtonUp(int32 button)
{
  auto iter = _buttonsPressed.find(button);
  if (iter == _buttonsPressed.end())
  {
    _buttonsPressed.insert(std::make_pair(button, false));
    return;
  }
  iter->second = false;
}

void InputManager::SetMouseScrollOffset(float32 offset)
{
  _mouseScrollOffset = offset;
}

bool InputManager::IsKeyDown(int32 key)
{
  auto iter = _keysPressed.find(key);
  if (iter == _keysPressed.end())
  {
    _keysPressed[key] = false;
    _keysPressed.insert(std::make_pair(key, false));
    return false;
  }
  return iter->second;
}

bool InputManager::IsButtonDown(int32 button)
{
  auto iter = _buttonsPressed.find(button);
  if (iter == _buttonsPressed.end())
  {
    _buttonsPressed.insert(std::make_pair(button, false));
    return false;
  }
  return iter->second;
}

MousePosition InputManager::GetMousePosition()
{
  if (_mousePositionCallback)
  {
    float64 xPosition = 0.0;
    float64 yPosition = 0.0;
    _mousePositionCallback(xPosition, yPosition);
    return MousePosition(xPosition, yPosition);
  }
  return MousePosition(-1.0, -1.0);
}

float32 InputManager::GetMouseScrollOffset() const
{
  return _mouseScrollOffset;
}

void InputManager::Poll()
{
  for (auto& keyPressed : _keysPressed)
  {
    int32 key = keyPressed.first;
    if (!IsKeyDown(key))
    {
      continue;
    }

    auto iter = _keyEvents.find(key);
    if (iter == _keyEvents.end())
    {
      continue;
    }

    for (auto& keyEvent : _keyEvents[key])
    {
      keyEvent();
    }
  }
}
}