#include "Application.h"

#ifdef  _WIN32
#define GLEW_STATIC 
#endif 

#include <SDL.h>

#include "..\Core\InputManager.h"
#include "..\Diagnostics\Logger.h"
#include "..\Rendering\Renderer.h"
#include "..\UI\UiManager.h"
#include "..\Utility\AssetManager.h"
#include "..\SceneManagement\SceneManager.h"

using namespace Diagnostics;
using namespace Platform;
using namespace Rendering;
using namespace UI;
using namespace Utility;
using namespace SceneManagement;

Application::~Application()
{
}

int32 Application::Run()
{
  if (!Initialize())
  {
    return -1;
  }
  _isRunning = true;
  OnStart();

  while (_isRunning)
  {
    SDL_Event sdlEvent;
    if (SDL_PollEvent(&sdlEvent))
    {
      switch (sdlEvent.type)
      {
      case SDL_QUIT:
        _isRunning = false;
        break;
      case SDL_KEYUP:
      case SDL_KEYDOWN:
        OnInput();
        break;
      }
    }

    uint32 dtMs = GetTickDuration();
    OnTick(dtMs);

    _renderer->PreRender();
    _renderer->DrawUI(_uiManager->GetPanelCollection());
    _renderer->DrawScene(_sceneManager->GetActiveScene());
    _sceneManager->UpdateActiveScene();

    SDL_GL_SwapWindow(_window);
  }

  SDL_DestroyWindow(_window);
  return 0;
}

Application::Application(const ApplicationDesc &desc) :
  _sceneManager(new SceneManager),
  _inputManager(new InputManager),
  _uiManager(new UIManager),
  _assetManager(new AssetManager("./../../ass/Textures/")),
  _desc(desc),
  _isRunning(false),
  _lastTimeInMs(0),
  _currentTimeInMs(0)
{
}

bool Application::Initialize()
{
  LogToConsole::ReportingLevel() = LogLevel::Verbose;

  if (SDL_Init(SDL_INIT_VIDEO) > 0)
  {
    std::string errorMessage = "Failed to initialize SDL: " + std::string(SDL_GetError());
    SDL_ClearError();
    return false;
  }

  _window = SDL_CreateWindow(_desc.Name.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, _desc.Width, _desc.Height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
  if (!_window)
  {
    std::string errorMessage = "Failed to create SDL window: " + std::string(SDL_GetError());
    SDL_ClearError();
    return false;
  }

  if (!_renderer->Initialize())
  {
    LOG_ERROR << "Could not intialize renderer";
    return false;
  }

  return true;
}

uint32 Application::GetTickDuration()
{
  _lastTimeInMs = _currentTimeInMs;
  uint32 durationInMs = SDL_GetTicks();
  _currentTimeInMs = _lastTimeInMs - durationInMs;
  return _currentTimeInMs;
}
