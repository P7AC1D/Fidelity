#pragma once
#include <functional>
#include <unordered_map>
#include <vector>

#include "../Core/Types.hpp"

class Vector2;

namespace Platform
{
class InputManager
{
public:
  InputManager();
  ~InputManager();

  void SetKeyDown(int32 key);
  void SetKeyUp(int32 key);
  void SetMouseButtonDown(int32 button);
  void SetMouseButtonUp(int32 button);
  void SetMouseScrollOffset(float32 offset);

  bool IsKeyDown(int32 key);
  bool IsButtonDown(int32 button);

  Vector2 GetMousePosition();
  float32 GetMouseScrollOffset() const;

  void Poll();

private:
  std::unordered_map<int32, bool> _keysPressed;
  std::unordered_map<int32, bool> _buttonsPressed;

  float32 _mouseScrollOffset;
};
}
