#include "Application.hpp"

#include "SDL.h"

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
    _sceneManager.UpdateScene(dtMs);
    
    OnTick(dtMs);
    SDL_GL_SwapWindow(_window);
  }
  
  SDL_DestroyWindow(_window);
  return 0;
}

Application::Application(const ApplicationDesc &desc):
_sceneManager(*SceneManager::Instance()),
_desc(desc),
_isRunning(false),
_lastTimeInMs(0),
_currentTimeInMs(0)
{
}

bool Application::Initialize()
{
  if (SDL_Init( SDL_INIT_VIDEO ) > 0)
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
  
  return true;
}

uint32 Application::GetTickDuration()
{
  _lastTimeInMs = _currentTimeInMs;
  uint32 durationInMs = SDL_GetTicks();
  _currentTimeInMs = _lastTimeInMs - durationInMs;
  return _currentTimeInMs;
}
