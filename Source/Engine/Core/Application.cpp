#include "Application.h"

#include <iostream>
#include <utility>
#include "../Input/EventDispatcher.hpp"
#include "../Input/InputHandler.hpp"
#include "../Rendering/Renderer.h"
#include "../SceneManagement/GenericNode.hpp"
#include "../SceneManagement/SceneNode.hpp"

static std::shared_ptr<InputHandler> INPUT_HANDLER = nullptr;
static std::shared_ptr<UiManager> DEBUG_UI = nullptr;
static int32 TICK_DURATION = 0;
static Vector2I MOUSE_POSITION = Vector2I(0);

void errorCallback(int error, const char* description)
{
  std::cerr << "GLFW Error: " << description << std::endl;
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  switch (action)
  {
  case GLFW_PRESS:
  {
    InputEvent inputEvent;
    inputEvent.Button = GlfwKeyToButton(key);
    inputEvent.ButtonEvent = ButtonEvent::Pressed;
    INPUT_HANDLER->Dispatch(inputEvent, TICK_DURATION);
    break;
  }
  case GLFW_RELEASE:
  {
    InputEvent inputEvent;
    inputEvent.Button = GlfwKeyToButton(key);
    inputEvent.ButtonEvent = ButtonEvent::Released;
    INPUT_HANDLER->Dispatch(inputEvent, TICK_DURATION);
    break;
  }
  case GLFW_REPEAT:
  {

  }
  }
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
  if (DEBUG_UI->HasMouseCapture())
  {
    return;
  }

  switch (action)
  {
  case GLFW_PRESS:
  {
    InputEvent inputEvent;
    inputEvent.AxisPos = MOUSE_POSITION;
    inputEvent.Button = GlfwMouseButtonToButton(button);
    inputEvent.ButtonEvent = ButtonEvent::Pressed;
    INPUT_HANDLER->Dispatch(inputEvent, TICK_DURATION);
    break;
  }
  case GLFW_RELEASE:
  {   
    InputEvent inputEvent;
    inputEvent.AxisPos = MOUSE_POSITION;
    inputEvent.Button = GlfwMouseButtonToButton(button);
    inputEvent.ButtonEvent = ButtonEvent::Released;
    INPUT_HANDLER->Dispatch(inputEvent, TICK_DURATION);
    break;
  }
  }
}

void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
{
  InputEvent inputEvent;
  inputEvent.Axis = Axis::MouseXY;
  inputEvent.AxisPos = Vector2I(xpos, ypos);
  inputEvent.AxisPosDelta = MOUSE_POSITION - inputEvent.AxisPos;
  INPUT_HANDLER->Dispatch(inputEvent, TICK_DURATION);

  MOUSE_POSITION = inputEvent.AxisPos;
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
  if (DEBUG_UI->HasMouseCapture())
  {
    return;
  }

  InputEvent inputEvent;
  inputEvent.Axis = Axis::MouseScrollXY;
  inputEvent.AxisPosDelta = Vector2I(xoffset, yoffset);
  INPUT_HANDLER->Dispatch(inputEvent, TICK_DURATION);
}

Application::~Application() 
{
  glfwDestroyWindow(_window);
  glfwTerminate();
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
    TICK_DURATION = dtMs;

    OnUpdate(dtMs);
    
    if (glfwWindowShouldClose(_window))
    {
      _isRunning = false;
      return 0;
    }   

		_sceneGraph->Update(dtMs);
    _sceneGraph->Draw(_renderer); //TODO Change name to SubmitDraw() or PrepareDraw() or something similar.
  	
		_renderer->DrawFrame();
		_debugUi->Update();		
    glfwSwapBuffers(_window);
    glfwPollEvents();
  }

  return 0;
}

Application::Application(ApplicationDesc desc) :
  _eventDispatcher(new EventDispatcher),
  _inputHandler(new InputHandler(*_eventDispatcher.get())),
  _sceneGraph(SceneNode::Create<GenericNode>()),
  _mouseFocus(true),
  _desc(std::move(desc))
{
  INPUT_HANDLER = _inputHandler;
}

float32 Application::GetAverageTickMs(int32 dtMs)
{
  static float32 currentDt = 0;
  static int32 dtSum = 0;
  static int32 count = 0;

  if (dtSum >= 500)
  {
    currentDt = dtSum / static_cast<float32>(count);
    dtSum = 0;
    count = 0;
  }
  else
  {
    dtSum += dtMs;
    count++;
  }  
  return currentDt;
}

float32 Application::GetAverageFps(int32 dtMs)
{
  return 1.0f / (GetAverageTickMs(dtMs) * 0.001f);
}

bool Application::Initialize()
{
  if (!glfwInit())
  {
    return false;
  }
  glfwSetErrorCallback(errorCallback);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  _window = glfwCreateWindow(_desc.Width, _desc.Height, _desc.Name.c_str(), nullptr, nullptr);
  if (!_window)
  {
    return false;
  }

  glfwSetWindowUserPointer(_window, this);
  glfwMakeContextCurrent(_window);
  glfwSetCursorPosCallback(_window, cursorPositionCallback);
  glfwSetMouseButtonCallback(_window, mouseButtonCallback);
  glfwSetScrollCallback(_window, scrollCallback);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    return false;
  }

	try
	{
		RendererDesc rendererDesc;
		rendererDesc.RenderWidth = _desc.Width;
		rendererDesc.RenderHeight = _desc.Height;
		_renderer.reset(new Renderer(rendererDesc));
	}
	catch (const std::exception& exception)
	{
#ifdef _DEBUG
		std::cout << exception.what();
#endif
		return false;
	}

  _debugUi.reset(new UiManager(_window));
	_debugUi->SetRenderer(_renderer);
  _debugUi->SetSceneGraph(_sceneGraph);
  DEBUG_UI = _debugUi;
  return true;
}

int32 Application::GetTickDuration()
{
  static int32 dtMs = 0;
  static float64 lastTimeInSeconds = 0;
  float64 currentTimeInSeconds = glfwGetTime();
  float64 dt = currentTimeInSeconds - lastTimeInSeconds;
  lastTimeInSeconds = currentTimeInSeconds;
  dtMs = dt * 1000; // convert to mili-seconds
  return dtMs;
}
