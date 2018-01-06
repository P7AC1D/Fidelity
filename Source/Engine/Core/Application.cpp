#include "Application.h"

#include <SDL.h>

#include "../Core/InputManager.h"
#include "../Rendering/Renderer.h"
#include "../UI/UIManager.h"
#include "../Utility/AssetManager.h"
#include "../SceneManagement/SceneManager.h"

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
  _renderer(new Renderer(desc.Width, desc.Height)),
  _uiManager(new UIManager),
  _assetManager(new AssetManager("./../../Assets/Textures/")),
  _desc(desc),
  _isRunning(false)
{
}

bool Application::Initialize()
{
  if (SDL_Init(SDL_INIT_VIDEO) > 0)
  {
    std::string errorMessage = "Failed to initialize SDL: " + std::string(SDL_GetError());
    SDL_ClearError();
    return false;
  }

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

  _window = SDL_CreateWindow(_desc.Name.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, _desc.Width, _desc.Height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
  if (!_window)
  {
    std::string errorMessage = "Failed to create SDL window: " + std::string(SDL_GetError());
    SDL_ClearError();
    return false;
  }
  
  _glContext = SDL_GL_CreateContext(_window);
  if (!_glContext)
  {
    std::string errorMessage = "Failed to create GL context: " + std::string(SDL_GetError());
    SDL_ClearError();
    return false;
  }

  if (!_renderer->Initialize())
  {
    return false;
  }

  return true;
}

uint32 Application::GetTickDuration()
{
  static uint32 dtMs = 0;
  static uint32 lastTimeInMs = 0;
  uint32 currentTimeInMs = SDL_GetTicks();
  dtMs = currentTimeInMs - lastTimeInMs;
  lastTimeInMs = currentTimeInMs;
  return dtMs;
}
