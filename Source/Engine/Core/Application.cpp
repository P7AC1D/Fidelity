#include "Application.h"

#include <SDL.h>

#include "../Input/EventDispatcher.hpp"
#include "../Input/InputHandler.hpp"
#include "../Rendering/Renderer.h"
#include "../UI/UIManager.h"
#include "../Utility/AssetManager.h"
#include "../SceneManagement/SceneManager.h"

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
    uint32 dtMs = GetTickDuration();
    OnUpdate(dtMs);
    
    SDL_Event sdlEvent;
    if (SDL_PollEvent(&sdlEvent))
    {
      switch (sdlEvent.type)
      {
        case SDL_QUIT:
          _isRunning = false;
          break;
        case SDL_WINDOWEVENT:
        {
          switch (sdlEvent.window.event)
          {
            case SDL_WINDOWEVENT_ENTER:
            {
              _mouseFocus = true;
              break;
            }
            case SDL_WINDOWEVENT_LEAVE:
            {
              _mouseFocus = false;
              break;
            }
          }
        }
        break;
        case SDL_KEYUP:
        {
          InputEvent inputEvent;
          inputEvent.Button = SDLToButton(sdlEvent.key.keysym.sym);
          inputEvent.ButtonEvent = ButtonEvent::Released;
          _inputHandler->Dispatch(inputEvent, dtMs);
          break;
        }
        case SDL_KEYDOWN:
        {
          InputEvent inputEvent;
          inputEvent.Button = SDLToButton(sdlEvent.key.keysym.sym);
          inputEvent.ButtonEvent = ButtonEvent::Pressed;
          _inputHandler->Dispatch(inputEvent, dtMs);
          break;
        }          
        case SDL_MOUSEBUTTONUP:
        {
          InputEvent inputEvent;
          inputEvent.Button = SDLToButton(sdlEvent.button.button);
          inputEvent.ButtonEvent = ButtonEvent::Released;
          _inputHandler->Dispatch(inputEvent, dtMs);
          break;
        }
        case SDL_MOUSEBUTTONDOWN:
        {
          InputEvent inputEvent;
          inputEvent.Button = SDLToButton(sdlEvent.button.button);
          inputEvent.ButtonEvent = ButtonEvent::Pressed;
          _inputHandler->Dispatch(inputEvent, dtMs);
          break;
        }
        case SDL_MOUSEMOTION:
        {
          if (_mouseFocus)
          {
            InputEvent inputEvent;
            inputEvent.Axis = Axis::MouseXY;
            inputEvent.AxisPos = Vector2i(sdlEvent.motion.x, sdlEvent.motion.y);
            inputEvent.AxisPosDelta = _cursorPosition - inputEvent.AxisPos;
            _inputHandler->Dispatch(inputEvent, dtMs);

            _cursorPosition = inputEvent.AxisPos;
          }
          break;
        }
        case SDL_MOUSEWHEEL:
        {
          InputEvent inputEvent;
          inputEvent.Axis = Axis::MouseScrollXY;
          inputEvent.AxisPosDelta = Vector2i(sdlEvent.wheel.x, sdlEvent.wheel.y);
          _inputHandler->Dispatch(inputEvent, dtMs);
          break;
        }
      }
    }   

    _renderer->PreRender();
    _renderer->DrawUI(_uiManager->GetPanelCollection());
    _renderer->DrawScene(_sceneManager->GetActiveScene());
    _sceneManager->UpdateActiveScene();

    SDL_GL_SwapWindow(_window);
    
    std::string title = _desc.Name + ": " + std::to_string(dtMs) + "ms";
    SDL_SetWindowTitle(_window, title.c_str());
  }

  SDL_DestroyWindow(_window);
  return 0;
}

Application::Application(const ApplicationDesc &desc) :
  _eventDispatcher(new EventDispatcher),
  _inputHandler(new InputHandler(*_eventDispatcher.get())),
  _sceneManager(new SceneManager),
  _renderer(new Renderer(desc.Width, desc.Height)),
  _uiManager(new UIManager),
  _assetManager(new AssetManager("./../../Assets/")),
  _desc(desc),
  _isRunning(false),
  _mouseFocus(true)
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

int32 Application::GetTickDuration()
{
  static int32 dtMs = 0;
  static int32 lastTimeInMs = 0;
  int32 currentTimeInMs = SDL_GetTicks();
  dtMs = currentTimeInMs - lastTimeInMs;
  lastTimeInMs = currentTimeInMs;
  return dtMs;
}
