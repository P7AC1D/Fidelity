#include "InputManager.h"

#include <SDL.h>

#include "../Maths/Vector2.hpp"

namespace Platform
{
InputManager::InputManager() :
  _mouseScrollOffset(0.0f)
{
}

InputManager::~InputManager()
{
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

Vector2 InputManager::GetMousePosition()
{
  int32 xPos = 0;
  int32 yPos = 0;
  SDL_GetMouseState(&xPos, &yPos);
  return Vector2(xPos, yPos);
}

float32 InputManager::GetMouseScrollOffset() const
{
  return _mouseScrollOffset;
}
}
