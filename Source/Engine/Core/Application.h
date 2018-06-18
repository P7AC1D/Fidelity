#pragma once
#include <functional>
#include <memory>
#include <string>

#include "../Core/Types.hpp"
#include "../Maths/Vector2I.hpp"
#include "../Utility/DebugUi.hpp"

class EventDispatcher;
class InputHandler;
class Renderer;
class SceneManager;

typedef void* SDL_GLContext;
struct SDL_Window;

struct ApplicationDesc
{
  std::string Name;
  uint32 Width;
  uint32 Height;
};

class Application
{
public:
  virtual void OnStart() {}
  virtual void OnUpdate(uint32 dtMs) {}

  std::string GetName() const { return _desc.Name; }
  uint32 GetWidth() const { return _desc.Width; }
  uint32 GetHeight() const { return _desc.Height; }

  int32 Run();

protected:
  Application(const ApplicationDesc& desc);

  float32 GetAverageTickMs(int32 dtMs);
  float32 GetAverageFps(int32 dtMs);

private:
  bool Initialize();
  int32 GetTickDuration();

protected:
  std::unique_ptr<EventDispatcher> _eventDispatcher;
  std::unique_ptr<InputHandler> _inputHandler;
  std::unique_ptr<SceneManager> _sceneManager;
	std::unique_ptr<DebugUi> _debugUi;
  std::shared_ptr<Renderer> _renderer;

private:
  bool _isRunning;
  bool _mouseFocus;
  SDL_Window* _window;
  SDL_GLContext _glContext;
  ApplicationDesc _desc;
  Vector2I _cursorPosition;
};
