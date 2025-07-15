#include "Sponza.h"

#include <iomanip>
#include <random>
#include <sstream>

#include "../Engine/Core/Scene.h"
#include "../Engine/Core/TransformComponent.h"
#include "../Engine/Rendering/CameraComponent.h"
#include "../Engine/Rendering/DrawableComponent.h"
#include "../Engine/Rendering/LightComponent.h"
#include "../Engine/Utility/ModelLoader.hpp"

SponzaModern::SponzaModern(const ApplicationDesc &desc) : Application(desc)
{
}

void SponzaModern::onStart()
{
  createCamera();
  createLights();
  loadSponzaModel();
}

void SponzaModern::createCamera()
{
  // Create main camera in the Scene
  GameObject &cameraObj = _scene.createGameObject("MainCamera");
  auto &transform = cameraObj.getComponent<TransformComponent>();
  auto &camera = cameraObj.addComponent<CameraComponent>();

  // Configure camera
  camera.setPerspective(Degree(67.67f), getWidth(), getHeight(), 0.1f, 500.0f);

  // Set position and rotation
  transform.setPosition(Vector3(-105.0f, 70.0f, 9.0f));
  transform.setRotation(Quaternion(Degree(59.552f), Degree(53.438f), Degree(53.802f)));

  // Store reference for component and for Application base camera
  _cameraComponent = &camera;
  _camera = &cameraObj;
}

void SponzaModern::createLights()
{
  // Directional Light
  {
    GameObject &dirLight = _scene.createGameObject("DirectionalLight");
    auto &transform = dirLight.getComponent<TransformComponent>();
    auto &light = dirLight.addComponent<LightComponent>();

    light.setLightType(LightComponentType::Directional)
        .setColour(Colour(244, 233, 155))
        .setIntensity(10.0f);

    transform.setRotation(Quaternion(Degree(36.139f), Degree(-72.174f), Degree(-30.861f)));
  }

  // Point Light 1 - Red
  {
    GameObject &light1 = _scene.createGameObject("PointLight1");
    auto &transform = light1.getComponent<TransformComponent>();
    auto &light = light1.addComponent<LightComponent>();

    light.setLightType(LightComponentType::Point)
        .setColour(Colour(150, 25, 25))
        .setRadius(70.0f)
        .setCastsShadows(true);

    transform.setPosition(Vector3(95.0f, 8.0f, 0.0f));
  }

  // Point Light 2 - Green
  {
    GameObject &light2 = _scene.createGameObject("PointLight2");
    auto &transform = light2.getComponent<TransformComponent>();
    auto &light = light2.addComponent<LightComponent>();

    light.setLightType(LightComponentType::Point)
        .setColour(Colour(25, 150, 25))
        .setRadius(70.0f);

    transform.setPosition(Vector3(-51.0f, 8.0f, 0.0f));
  }

  // Point Light 3 - Blue
  {
    GameObject &light3 = _scene.createGameObject("PointLight3");
    auto &transform = light3.getComponent<TransformComponent>();
    auto &light = light3.addComponent<LightComponent>();

    light.setLightType(LightComponentType::Point)
        .setColour(Colour(25, 25, 100))
        .setRadius(70.0f);

    transform.setPosition(Vector3(12.0f, 8.0f, 0.0f));
  }
}

void SponzaModern::loadSponzaModel()
{
  // Load the Sponza model and integrate via ModelLoader
  GameObject &modelRoot = ModelLoader::fromFile(_scene, "./Models/sponza_pbr/sponza.obj", true);
  // Apply uniform scale to match previous setup
  modelRoot.transform().setScale(Vector3(0.1f, 0.1f, 0.1f));
}

void SponzaModern::onUpdate(uint32 dtMs)
{
  if (!_cameraComponent)
    return;

  Vector2I mousePosDelta = _lastMousePos - _currentMousePos;

  // Camera movement with modern component system
  if (_inputHandler->isButtonPressed(Button::Key_W))
  {
    float32 deltaX = static_cast<float32>(dtMs) * (_inputHandler->isButtonPressed(Button::Key_LShift) ? CAMERA_MOVE_SPRINT_FACTOR : CAMERA_MOVE_FACTOR);
    translateCamera(deltaX, 0.0f);
  }
  else if (_inputHandler->isButtonPressed(Button::Key_S))
  {
    float32 deltaX = static_cast<float32>(dtMs) * (_inputHandler->isButtonPressed(Button::Key_LShift) ? CAMERA_MOVE_SPRINT_FACTOR : CAMERA_MOVE_FACTOR);
    translateCamera(-deltaX, 0.0f);
  }

  if (_inputHandler->isButtonPressed(Button::Key_D))
  {
    float32 deltaY = static_cast<float32>(dtMs) * (_inputHandler->isButtonPressed(Button::Key_LShift) ? CAMERA_MOVE_SPRINT_FACTOR : CAMERA_MOVE_FACTOR);
    translateCamera(0.0f, deltaY);
  }
  else if (_inputHandler->isButtonPressed(Button::Key_A))
  {
    float32 deltaY = static_cast<float32>(dtMs) * (_inputHandler->isButtonPressed(Button::Key_LShift) ? CAMERA_MOVE_SPRINT_FACTOR : CAMERA_MOVE_FACTOR);
    translateCamera(0.0f, -deltaY);
  }

  // Mouse look (existing functionality)
  if (_inputHandler->isButtonPressed(Button::Button_RMouse))
  {
    fpsCameraLook(mousePosDelta[0], mousePosDelta[1], dtMs);
  }

  // Arrow key look (new functionality)
  handleArrowKeyLook(dtMs);
}

void SponzaModern::translateCamera(float32 deltaX, float32 deltaY)
{
  if (!_camera || !_cameraComponent)
    return;

  // Update modern component system
  auto &transform = _camera->getComponent<TransformComponent>();

  // Get camera's current forward and right vectors
  Vector3 forward = _cameraComponent->getWorldForward();
  Vector3 right = _cameraComponent->getWorldRight();

  // Calculate movement vector
  Vector3 movement = forward * deltaX + right * deltaY;

  // Apply movement to modern system
  Vector3 currentPos = transform.getPosition();
  Vector3 newPos = currentPos + movement;
  transform.setPosition(newPos);
}

void SponzaModern::fpsCameraLook(float32 deltaX, float32 deltaY, uint32 dtMs)
{
  if (!_camera || !_cameraComponent)
    return;

  // Simple FPS-style camera look for modern system
  auto &transform = _camera->getComponent<TransformComponent>();

  // Calculate yaw around world up and pitch around camera right
  Radian yawRadian(deltaX * CAMERA_LOOK_SENSITIVITY);
  Radian pitchRadian(deltaY * CAMERA_LOOK_SENSITIVITY);

  // Get current pitch to check limits before applying new rotation
  float32 currentPitch = extractPitchFromQuaternion(transform.getRotation());
  float32 newPitch = currentPitch + pitchRadian.InRadians() * (180.0f / Math::Pi); // Convert to degrees

  // Clamp pitch to prevent gimbal lock and over-rotation
  if (newPitch > CAMERA_MAX_PITCH_DEGREES)
  {
    pitchRadian = Radian((CAMERA_MAX_PITCH_DEGREES - currentPitch * (180.0f / Math::Pi)) * (Math::Pi / 180.0f));
  }
  else if (newPitch < -CAMERA_MAX_PITCH_DEGREES)
  {
    pitchRadian = Radian((-CAMERA_MAX_PITCH_DEGREES - currentPitch * (180.0f / Math::Pi)) * (Math::Pi / 180.0f));
  }

  // Construct quaternions from axis-angle: (axis, angle)
  Quaternion yawQuat(Vector3::Up, yawRadian);
  Vector3 camRight = _cameraComponent->getWorldRight();
  Quaternion pitchQuat(camRight, pitchRadian);
  Quaternion newRot = yawQuat * pitchQuat * transform.getRotation();
  transform.setRotation(newRot);
}

void SponzaModern::handleArrowKeyLook(uint32 dtMs)
{
  if (!_camera || !_cameraComponent)
    return;

  // Calculate arrow key deltas based on pressed keys
  float32 deltaX = 0.0f;
  float32 deltaY = 0.0f;

  // Horizontal rotation (yaw) - Left/Right arrows
  if (_inputHandler->isButtonPressed(Button::Key_Left))
  {
    deltaX = -CAMERA_ARROW_LOOK_SENSITIVITY * static_cast<float32>(dtMs) / 1000.0f;
  }
  else if (_inputHandler->isButtonPressed(Button::Key_Right))
  {
    deltaX = CAMERA_ARROW_LOOK_SENSITIVITY * static_cast<float32>(dtMs) / 1000.0f;
  }

  // Vertical rotation (pitch) - Up/Down arrows
  if (_inputHandler->isButtonPressed(Button::Key_Up))
  {
    deltaY = -CAMERA_ARROW_LOOK_SENSITIVITY * static_cast<float32>(dtMs) / 1000.0f;
  }
  else if (_inputHandler->isButtonPressed(Button::Key_Down))
  {
    deltaY = CAMERA_ARROW_LOOK_SENSITIVITY * static_cast<float32>(dtMs) / 1000.0f;
  }

  // Apply camera rotation if any arrow keys are pressed
  if (deltaX != 0.0f || deltaY != 0.0f)
  {
    // Reuse the existing fpsCameraLook method for consistent behavior
    fpsCameraLook(deltaX, deltaY, dtMs);
  }
}

float32 SponzaModern::extractPitchFromQuaternion(const Quaternion &rotation) const
{
  // Extract pitch using the same formula as Quaternion::Pitch()
  // but return in radians for internal calculations
  const float32 y = 2.0f * (rotation.Y * rotation.Z + rotation.W * rotation.X);
  const float32 x = rotation.W * rotation.W - rotation.X * rotation.X - rotation.Y * rotation.Y + rotation.Z * rotation.Z;

  if (y == 0.0f && x == 0.0f)
  {
    return 2.0f * std::atan2(rotation.X, rotation.W);
  }
  return std::atan2(y, x);
}
