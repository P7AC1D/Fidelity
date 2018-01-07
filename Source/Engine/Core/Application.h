#pragma once
#include <functional>
#include <memory>
#include <string>

#include "../Core/Types.hpp"

namespace Rendering
{
class Renderer;
class RenderContext;
}

namespace SceneManagement
{
class SceneManager;
}

namespace UI
{
class UIManager;
}

namespace Utility
{
class AssetManager;
}

class EventDispatcher;
class InputHandler;

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
  virtual ~Application();

  virtual void OnStart() {}
  virtual void OnInput() {}
  virtual void OnDraw() {}
  virtual void OnTick(uint32 dtMs) {}

  std::string GetName() const { return _desc.Name; }
  uint32 GetWidth() const { return _desc.Width; }
  uint32 GetHeight() const { return _desc.Height; }

  int32 Run();

protected:
  Application(const ApplicationDesc& desc);

private:
  Application(Application&) = delete;
  Application(Application&&) = delete;
  Application& operator=(Application&) = delete;
  Application& operator= (Application&&) = delete;

  bool Initialize();
  uint32 GetTickDuration();

protected:
  std::unique_ptr<EventDispatcher> _eventDispatcher;
  std::unique_ptr<InputHandler> _inputHandler;
  std::unique_ptr<SceneManagement::SceneManager> _sceneManager;
  std::unique_ptr<Rendering::Renderer> _renderer;
  std::unique_ptr<UI::UIManager> _uiManager;
  std::unique_ptr<Utility::AssetManager> _assetManager;

private:
  bool _isRunning;
  SDL_Window* _window;
  SDL_GLContext _glContext;
  ApplicationDesc _desc;
};
