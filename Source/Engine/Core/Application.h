#pragma once
#include <functional>
#include <memory>
#include <string>

#include "../Core/Types.hpp"

struct GLFWwindow;

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

namespace Platform
{
class InputManager;
}

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
  std::unique_ptr<SceneManagement::SceneManager> _sceneManager;
  std::unique_ptr<Rendering::Renderer> _renderer;
  std::unique_ptr<Platform::InputManager> _inputManager;
  std::unique_ptr<UI::UIManager> _uiManager;
  std::unique_ptr<Utility::AssetManager> _assetManager;

private:
  ApplicationDesc _desc;
  SDL_Window* _window;
  bool _isRunning;
  uint32 _lastTimeInMs;
  uint32 _currentTimeInMs;
};