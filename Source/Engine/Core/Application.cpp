#include "Application.h"

#include <iostream>
#include <utility>
#include "../RenderApi/GL/GLRenderDevice.hpp"
#include "../RenderApi/Texture.hpp"
#include "../Utility/TextureLoader.hpp"
#include "InputHandler.h"

static std::shared_ptr<InputHandler> INPUT_HANDLER = nullptr;
static std::shared_ptr<UiManager> DEBUG_UI = nullptr;

void errorCallback(int error, const char *description)
{
  std::cerr << "GLFW Error: " << description << std::endl;
}

void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
  switch (action)
  {
  case GLFW_PRESS:
  {
    INPUT_HANDLER->setButtonState(GlfwKeyToButton(key), ButtonState::Pressed);
    break;
  }
  case GLFW_RELEASE:
  {
    INPUT_HANDLER->setButtonState(GlfwKeyToButton(key), ButtonState::Released);
    break;
  }
  case GLFW_REPEAT:
  {
    INPUT_HANDLER->setButtonState(GlfwKeyToButton(key), ButtonState::Pressed);
    break;
  }
  }
}

void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
  if (DEBUG_UI->hasMouseCapture())
  {
    return;
  }

  switch (action)
  {
  case GLFW_PRESS:
  {
    INPUT_HANDLER->setButtonState(GlfwMouseButtonToButton(button), ButtonState::Pressed);
    break;
  }
  case GLFW_RELEASE:
  {
    INPUT_HANDLER->setButtonState(GlfwMouseButtonToButton(button), ButtonState::Released);
    break;
  }
  }
}

void cursorPositionCallback(GLFWwindow *window, float64 xpos, float64 ypos)
{
  INPUT_HANDLER->setAxisState(Axis::MouseXY, Vector2I(static_cast<int32>(xpos), static_cast<int32>(ypos)));
}

void scrollCallback(GLFWwindow *window, float64 xoffset, float64 yoffset)
{
  if (DEBUG_UI->hasMouseCapture())
  {
    return;
  }

  INPUT_HANDLER->setAxisState(Axis::MouseScrollXY, Vector2I(static_cast<int32>(xoffset), static_cast<int32>(yoffset)));
}

Button GlfwKeyToButton(int32 button)
{
  switch (button)
  {
  case GLFW_KEY_ESCAPE:
    return Button::Key_Escape;
  case GLFW_KEY_F1:
    return Button::Key_F1;
  case GLFW_KEY_F2:
    return Button::Key_F2;
  case GLFW_KEY_F3:
    return Button::Key_F3;
  case GLFW_KEY_F4:
    return Button::Key_F4;
  case GLFW_KEY_F5:
    return Button::Key_F5;
  case GLFW_KEY_F6:
    return Button::Key_F6;
  case GLFW_KEY_F7:
    return Button::Key_F7;
  case GLFW_KEY_F8:
    return Button::Key_F8;
  case GLFW_KEY_F9:
    return Button::Key_F9;
  case GLFW_KEY_F10:
    return Button::Key_F10;
  case GLFW_KEY_F11:
    return Button::Key_F11;
  case GLFW_KEY_F12:
    return Button::Key_F12;
  case GLFW_KEY_GRAVE_ACCENT:
    return Button::Key_BackQuote;
  case GLFW_KEY_1:
    return Button::Key_1;
  case GLFW_KEY_2:
    return Button::Key_2;
  case GLFW_KEY_3:
    return Button::Key_3;
  case GLFW_KEY_4:
    return Button::Key_4;
  case GLFW_KEY_5:
    return Button::Key_5;
  case GLFW_KEY_6:
    return Button::Key_6;
  case GLFW_KEY_7:
    return Button::Key_7;
  case GLFW_KEY_8:
    return Button::Key_8;
  case GLFW_KEY_9:
    return Button::Key_9;
  case GLFW_KEY_0:
    return Button::Key_0;
  case GLFW_KEY_MINUS:
    return Button::Key_Minus;
  case GLFW_KEY_EQUAL:
    return Button::Key_Equals;
  case GLFW_KEY_BACKSPACE:
    return Button::Key_BackSpace;
  case GLFW_KEY_TAB:
    return Button::Key_Tab;
  case GLFW_KEY_Q:
    return Button::Key_Q;
  case GLFW_KEY_W:
    return Button::Key_W;
  case GLFW_KEY_E:
    return Button::Key_E;
  case GLFW_KEY_R:
    return Button::Key_R;
  case GLFW_KEY_T:
    return Button::Key_T;
  case GLFW_KEY_Y:
    return Button::Key_Y;
  case GLFW_KEY_U:
    return Button::Key_U;
  case GLFW_KEY_I:
    return Button::Key_I;
  case GLFW_KEY_O:
    return Button::Key_O;
  case GLFW_KEY_P:
    return Button::Key_P;
  case GLFW_KEY_LEFT_BRACKET:
    return Button::Key_LBracket;
  case GLFW_KEY_RIGHT_BRACKET:
    return Button::Key_RBracket;
  case GLFW_KEY_BACKSLASH:
    return Button::Key_BackSlash;
  case GLFW_KEY_CAPS_LOCK:
    return Button::Key_CapsLock;
  case GLFW_KEY_A:
    return Button::Key_A;
  case GLFW_KEY_S:
    return Button::Key_S;
  case GLFW_KEY_D:
    return Button::Key_D;
  case GLFW_KEY_F:
    return Button::Key_F;
  case GLFW_KEY_G:
    return Button::Key_G;
  case GLFW_KEY_H:
    return Button::Key_H;
  case GLFW_KEY_J:
    return Button::Key_J;
  case GLFW_KEY_K:
    return Button::Key_K;
  case GLFW_KEY_L:
    return Button::Key_L;
  case GLFW_KEY_SEMICOLON:
    return Button::Key_SemiColon;
  case GLFW_KEY_APOSTROPHE:
    return Button::Key_Apostrophe;
  case GLFW_KEY_ENTER:
    return Button::Key_Return;
  case GLFW_KEY_LEFT_SHIFT:
    return Button::Key_LShift;
  case GLFW_KEY_Z:
    return Button::Key_Z;
  case GLFW_KEY_X:
    return Button::Key_X;
  case GLFW_KEY_C:
    return Button::Key_C;
  case GLFW_KEY_V:
    return Button::Key_V;
  case GLFW_KEY_B:
    return Button::Key_B;
  case GLFW_KEY_N:
    return Button::Key_N;
  case GLFW_KEY_M:
    return Button::Key_M;
  case GLFW_KEY_COMMA:
    return Button::Key_Comma;
  case GLFW_KEY_PERIOD:
    return Button::Key_Period;
  case GLFW_KEY_SLASH:
    return Button::Key_ForwardSlash;
  case GLFW_KEY_RIGHT_SHIFT:
    return Button::Key_RShift;
  case GLFW_KEY_LEFT_CONTROL:
    return Button::Key_LCtrl;
  case GLFW_KEY_LEFT_ALT:
    return Button::Key_LAlt;
  case GLFW_KEY_SPACE:
    return Button::Key_Space;
  case GLFW_KEY_RIGHT_ALT:
    return Button::Key_RAlt;
  case GLFW_KEY_RIGHT_CONTROL:
    return Button::Key_RCtrl;
  case GLFW_KEY_INSERT:
    return Button::Key_Insert;
  case GLFW_KEY_HOME:
    return Button::Key_Home;
  case GLFW_KEY_PAGE_UP:
    return Button::Key_PageUp;
  case GLFW_KEY_DELETE:
    return Button::Key_Delete;
  case GLFW_KEY_END:
    return Button::Key_End;
  case GLFW_KEY_PAGE_DOWN:
    return Button::Key_PageDown;
  case GLFW_KEY_UP:
    return Button::Key_Up;
  case GLFW_KEY_LEFT:
    return Button::Key_Left;
  case GLFW_KEY_DOWN:
    return Button::Key_Down;
  case GLFW_KEY_RIGHT:
    return Button::Key_Right;
  case GLFW_KEY_KP_DIVIDE:
    return Button::Key_NumDivide;
  case GLFW_KEY_KP_MULTIPLY:
    return Button::Key_NumMultiply;
  case GLFW_KEY_KP_SUBTRACT:
    return Button::Key_NumMinus;
  case GLFW_KEY_KP_1:
    return Button::Key_Num1;
  case GLFW_KEY_KP_2:
    return Button::Key_Num2;
  case GLFW_KEY_KP_3:
    return Button::Key_Num3;
  case GLFW_KEY_KP_4:
    return Button::Key_Num4;
  case GLFW_KEY_KP_5:
    return Button::Key_Num5;
  case GLFW_KEY_KP_6:
    return Button::Key_Num6;
  case GLFW_KEY_KP_7:
    return Button::Key_Num7;
  case GLFW_KEY_KP_8:
    return Button::Key_Num8;
  case GLFW_KEY_KP_9:
    return Button::Key_Num9;
  case GLFW_KEY_KP_0:
    return Button::Key_Num0;
  case GLFW_KEY_KP_ADD:
    return Button::Key_NumPlus;
  case GLFW_KEY_KP_ENTER:
    return Button::Key_NumReturn;
    // case GLFW_KEY_KP_PERIOD: return Button::Key_NumPeriod;
  default:
    return Button::Unknown;
  }
}

Button GlfwMouseButtonToButton(int32 button)
{
  switch (button)
  {
  case GLFW_MOUSE_BUTTON_1:
    return Button::Button_LMouse;
  case GLFW_MOUSE_BUTTON_2:
    return Button::Button_RMouse;
  case GLFW_MOUSE_BUTTON_3:
    return Button::Button_Mouse3;
  case GLFW_MOUSE_BUTTON_4:
    return Button::Button_Mouse4;
  case GLFW_MOUSE_BUTTON_5:
    return Button::Button_Mouse5;
  default:
    throw std::runtime_error("Unsupported GLFW button");
  }
}

Application::~Application()
{
  glfwDestroyWindow(_window);
  glfwTerminate();
}

int32 Application::run()
{
  try
  {
    if (!initialize())
    {
      return -1;
    }
    _isRunning = true;
    onStart();

    while (_isRunning)
    {
      uint32 dtMs = getTickDuration();

      if (glfwWindowShouldClose(_window))
      {
        _isRunning = false;
        return 0;
      }

      _scene.update(dtMs);
      _scene.drawFrame();
      _debugUi->update(_scene);

      onUpdate(dtMs);

      glfwSwapBuffers(_window);
      glfwPollEvents();
    }
  }
  catch (const std::exception &e)
  {
    std::cerr << e.what() << '\n';
  }

  return 0;
}

Application::Application(ApplicationDesc desc) : _inputHandler(new InputHandler()),
                                                 _mouseFocus(true),
                                                 _desc(std::move(desc)),
                                                 _cameraTarget(Vector3::Zero)
{
  INPUT_HANDLER = _inputHandler;
}

float32 Application::getAverageTickMs(int32 dtMs)
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

float32 Application::getAverageFps(int32 dtMs)
{
  return 1.0f / (getAverageTickMs(dtMs) * 0.001f);
}

// TODO: Why is this here again? Needs cleanup.
std::shared_ptr<Texture> Application::loadTextureFromFile(const std::string &path, bool generateMips, bool sRgb)
{
  return TextureLoader::loadFromFile2D(_renderDevice, path, generateMips, sRgb);
}

bool Application::initialize()
{
  if (!glfwInit())
  {
    return false;
  }
  glfwSetErrorCallback(errorCallback);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  // glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE);

  _window = glfwCreateWindow(_desc.Width, _desc.Height, _desc.Name.c_str(), nullptr, nullptr);
  if (!_window)
  {
    return false;
  }

  int fbWidth, fbHeight;
  glfwGetFramebufferSize(_window, &fbWidth, &fbHeight);

  glfwSetWindowUserPointer(_window, this);
  glfwMakeContextCurrent(_window);
  glfwSetCursorPosCallback(_window, cursorPositionCallback);
  glfwSetMouseButtonCallback(_window, mouseButtonCallback);
  glfwSetKeyCallback(_window, keyCallback);
  glfwSetScrollCallback(_window, scrollCallback);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    return false;
  }

  try
  {
    RenderDeviceDesc renderDeviceDesc;
    renderDeviceDesc.RenderWidth = fbWidth;
    renderDeviceDesc.RenderHeight = fbHeight;
    _renderDevice.reset(new GLRenderDevice(renderDeviceDesc));
  }
  catch (const std::exception &exception)
  {
    std::cerr << "Renderdevice exception thrown. " << exception.what();
    return false;
  }

  if (!_scene.init(Vector2I(fbWidth, fbHeight), _renderDevice))
  {
    std::cerr << "Failed to initialize scene." << std::endl;
    return false;
  }

  _debugUi.reset(new UiManager(_window));
  _debugUi->initialize(_renderDevice);
  DEBUG_UI = _debugUi;
  return true;
}

int32 Application::getTickDuration()
{
  static int32 dtMs = 0;
  static float64 lastTimeInSeconds = 0;
  float64 currentTimeInSeconds = glfwGetTime();
  float64 dt = currentTimeInSeconds - lastTimeInSeconds;
  lastTimeInSeconds = currentTimeInSeconds;
  dtMs = dt * 1000; // convert to mili-seconds
  return dtMs;
}

void Application::fpsCameraLook(int32 deltaX, int32 deltaY, uint32 dtMs)
{
  if (deltaX == 0 && deltaY == 0)
  {
    return;
  }

  Transform &cameraTransform = _camera->transform();
  float32 velocity(CAMERA_LOOK_SENSITIVITY * static_cast<float32>(dtMs));
  Quaternion qPitch(Vector3(1.0f, 0.0f, 0.0f), -velocity * deltaY);
  Quaternion qYaw(Vector3(0.0f, 1.0f, 0.0f), -velocity * deltaX);

  cameraTransform.setRotation(qPitch * cameraTransform.getRotation() * qYaw);
  _cameraTarget = cameraTransform.getForward();
}

void Application::rotateCamera(int32 deltaX, int32 deltaY, uint32 dtMs)
{
  if (deltaX == 0 && deltaY == 0)
  {
    return;
  }

  Transform &cameraTransform = _camera->transform();
  float32 velocity(CAMERA_ROTATION_FACTOR * static_cast<float32>(dtMs));
  Quaternion pitch(cameraTransform.getRight(), velocity * deltaY);
  Quaternion yaw(cameraTransform.getUp(), velocity * deltaX);
  Quaternion rotation(pitch * yaw);
  Vector3 newPosition(rotation.Rotate(cameraTransform.getPosition()));
  cameraTransform.lookAt(newPosition, cameraTransform.getForward());
}

void Application::zoomCamera(int32 delta, uint32 dtMs)
{
  if (delta == 0)
  {
    return;
  }

  Transform &cameraTransform = _camera->transform();
  Vector3 cameraForward = cameraTransform.getForward();
  Vector3 cameraPostion = cameraTransform.getPosition();

  float32 distanceToTarget = Vector3::Length(cameraPostion - _cameraTarget);
  float32 velocity(CAMERA_ZOOM_FACTOR * distanceToTarget * static_cast<float32>(dtMs));
  Vector3 newPosition(cameraPostion + cameraForward * velocity * -delta);

  Vector3 cameraToOldPos(cameraPostion - _cameraTarget);
  Vector3 cameraToNewPos(newPosition - _cameraTarget);
  if (Vector3::Dot(cameraToNewPos, cameraToOldPos) < 0.0f)
  {
    newPosition = _cameraTarget + cameraForward * 0.1f;
  }
  cameraTransform.lookAt(newPosition, _cameraTarget);
}

void Application::translateCamera(float32 forward, float32 right)
{
  if (forward == 0.0f && right == 0.0f)
  {
    return;
  }

  Transform &cameraTransform = _camera->transform();
  Vector3 cameraForward = cameraTransform.getForward();
  Vector3 cameraRight = cameraTransform.getRight();
  cameraTransform.translate(-cameraForward * forward + cameraRight * right);
}