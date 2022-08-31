#pragma once
#include <functional>
#include <memory>
#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "../Core/Types.hpp"
#include "../UI/UiManager.hpp"
#include "Scene.h"
#include "Maths.h"

class EventDispatcher;
class InputHandler;
class Renderer;

struct ApplicationDesc
{
  std::string Name;
  uint32 Width;
  uint32 Height;
};

class Application
{
public:
  virtual ~Application();
  virtual void OnStart() {}
  virtual void OnUpdate(uint32 dtMs) {}

  std::string GetName() const { return _desc.Name; }
  uint32 GetWidth() const { return _desc.Width; }
  uint32 GetHeight() const { return _desc.Height; }

  int32 Run();

protected:
  Application(ApplicationDesc desc);

  float32 GetAverageTickMs(int32 dtMs);
  float32 GetAverageFps(int32 dtMs);

private:
  bool Initialize();
  int32 GetTickDuration();

protected:
  std::unique_ptr<EventDispatcher> _eventDispatcher;
  std::shared_ptr<InputHandler> _inputHandler;
  std::shared_ptr<UiManager> _debugUi;
  std::shared_ptr<Renderer> _renderer;
  Scene _scene;

private:
  bool _isRunning{};
  bool _mouseFocus;
  ApplicationDesc _desc;
  Vector2I _cursorPosition;
  GLFWwindow *_window;
};
