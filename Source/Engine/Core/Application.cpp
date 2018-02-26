#include "Application.h"

#include <SDL.h>

#include "../Input/EventDispatcher.hpp"
#include "../Input/InputHandler.hpp"
#include "../Overlay/GuiSystem.hpp"
#include "../Rendering/Renderer.h"
#include "../Utility/AssetManager.h"
#include "../SceneManagement/SceneManager.h"

using namespace Rendering;

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
    while (SDL_PollEvent(&sdlEvent))
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
          GuiSystem::Get()->OnEvent(inputEvent);
          break;
        }
        case SDL_KEYDOWN:
        {
          InputEvent inputEvent;
          inputEvent.Button = SDLToButton(sdlEvent.key.keysym.sym);
          inputEvent.ButtonEvent = ButtonEvent::Pressed;
          _inputHandler->Dispatch(inputEvent, dtMs);
          GuiSystem::Get()->OnEvent(inputEvent);
          break;
        }          
        case SDL_MOUSEBUTTONUP:
        {
          InputEvent inputEvent;
          inputEvent.AxisPos = Vector2I(sdlEvent.button.x, sdlEvent.button.y);
          inputEvent.Button = SDLToButton(sdlEvent.button.button);
          inputEvent.ButtonEvent = ButtonEvent::Released;
          _inputHandler->Dispatch(inputEvent, dtMs);
          GuiSystem::Get()->OnEvent(inputEvent);
          break;
        }
        case SDL_MOUSEBUTTONDOWN:
        {
          InputEvent inputEvent;
          inputEvent.AxisPos = Vector2I(sdlEvent.button.x, sdlEvent.button.y);
          inputEvent.Button = SDLToButton(sdlEvent.button.button);
          inputEvent.ButtonEvent = ButtonEvent::Pressed;
          _inputHandler->Dispatch(inputEvent, dtMs);
          GuiSystem::Get()->OnEvent(inputEvent);
          break;
        }
        case SDL_MOUSEMOTION:
        {
          InputEvent inputEvent;
          inputEvent.Axis = Axis::MouseXY;
          inputEvent.AxisPos = Vector2I(sdlEvent.motion.x, sdlEvent.motion.y);
          inputEvent.AxisPosDelta = _cursorPosition - inputEvent.AxisPos;
          _inputHandler->Dispatch(inputEvent, dtMs);
          GuiSystem::Get()->OnEvent(inputEvent);

          _cursorPosition = inputEvent.AxisPos;
          break;
        }
        case SDL_MOUSEWHEEL:
        {
          InputEvent inputEvent;
          inputEvent.Axis = Axis::MouseScrollXY;
          inputEvent.AxisPosDelta = Vector2I(sdlEvent.wheel.x, sdlEvent.wheel.y);
          _inputHandler->Dispatch(inputEvent, dtMs);
          GuiSystem::Get()->OnEvent(inputEvent);
          break;
        }
      }
    }   

    _sceneManager->UpdateScene(dtMs);
    GuiSystem::Get()->Draw();

    SDL_GL_SwapWindow(_window);
  }

  SDL_DestroyWindow(_window);
  return 0;
}

Application::Application(const ApplicationDesc &desc) :
  _eventDispatcher(new EventDispatcher),
  _inputHandler(new InputHandler(*_eventDispatcher.get())),
  _isRunning(false),
  _mouseFocus(true),
  _desc(desc)
{
  _sceneManager.reset(new SceneManager());
}

float32 Application::GetAverageTickMs(int32 dtMs)
{
  static int dtSum = 0;
  static int dtCount = 0;

  float32 avgTick = dtSum / static_cast<float32>(dtCount);
  if (dtSum > 500)
  {
    dtSum = 0;
    dtCount = 0;
  }
  else
  {
    dtSum += dtMs;
    dtCount++;
  }
  return avgTick;
}

float32 Application::GetAverageFps(int32 dtMs)
{
  return 1.0f / (GetAverageTickMs(dtMs) * 0.001f);
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

  auto renderer = Renderer::Get();
  renderer->SetRenderDimensions(_desc.Width, _desc.Height);
  if (!renderer->Initialize())
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
