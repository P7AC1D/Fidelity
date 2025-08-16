#pragma once
#include <functional>
#include <memory>
#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "../Core/Types.hpp"
#include "../UI/UiManager.hpp"
#include "Scene.h"
#include "Maths.h"
#include "GpuPreference.h"

class Renderer;
class RenderDevice;
class Texture;
class InputHandler;

static const float32 CAMERA_LOOK_SENSITIVITY = 0.001f;
static const float32 CAMERA_MOVE_FACTOR = 0.10f;
static const float32 CAMERA_MOVE_SPRINT_FACTOR = 1.0f;
static const float32 CAMERA_MAX_PITCH_DEGREES = 89.0f; // Prevent full gimbal lock

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
  virtual void onStart() {}
  virtual void onUpdate([[maybe_unused]] uint32 dtMs) {}

  std::string getName() const { return _desc.Name; }
  uint32 getWidth() const { return _desc.Width; }
  uint32 getHeight() const { return _desc.Height; }

  int32 run();

  // Request application shutdown (safe to call from callbacks)
  void requestShutdown();

protected:
  Application(ApplicationDesc desc);

  float32 getAverageTickMs(int32 dtMs);
  float32 getAverageFps(int32 dtMs);

  // TODO: Move this elsewhere
  std::shared_ptr<Texture> loadTextureFromFile(const std::string &path, bool generateMips, bool sRgb);

private:
  bool initialize();
  int32 getTickDuration();

protected:
  void fpsCameraLook(int32 deltaX, int32 deltaY, uint32 dtMs);
  void translateCamera(float32 forward, float32 right);

  // Helper function to extract pitch from quaternion for clamping
  float32 extractPitchFromQuaternion(const Quaternion &rotation) const;

  Vector2I _lastMousePos, _currentMousePos;
  Vector3 _cameraTarget;
  GameObject *_camera;

  std::shared_ptr<InputHandler> _inputHandler;
  std::shared_ptr<UiManager> _debugUi;
  std::shared_ptr<Renderer> _renderer;
  Scene _scene;

private:
  bool _isRunning{};
  bool _mouseFocus;
  ApplicationDesc _desc;
  Vector2I _cursorPosition;
  GLFWwindow *_window;
  std::shared_ptr<RenderDevice> _renderDevice;
  float32 _windowToFramebufferRatio;
};
