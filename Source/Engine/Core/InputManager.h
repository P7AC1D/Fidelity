#pragma once
#include <functional>
#include <unordered_map>
#include <vector>

#include "../Core/Types.hpp"

namespace Platform
{
struct MousePosition
{
  MousePosition(float64 x, float64 y) :
    XPosition(x),
    YPosition(y)
  {}

  float64 XPosition;
  float64 YPosition;
};

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

  MousePosition GetMousePosition();
  float32 GetMouseScrollOffset() const;

  void Poll();

private:
  std::unordered_map<int32, bool> _keysPressed;
  std::unordered_map<int32, bool> _buttonsPressed;

  float32 _mouseScrollOffset;
};
}
