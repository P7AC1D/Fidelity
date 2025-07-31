#include "Sponza.h"

#include <iomanip>
#include <random>
#include <sstream>
#include <cmath>

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

  // Configure camera with appropriate far plane for the scaled scene
  camera.setPerspective(Degree(67.67f), getWidth(), getHeight(), 0.1f, 1000.0f);

  // Set position and rotation - original values should work now with proper scaling
  transform.setPosition(Vector3(-105.0f, 70.0f, 9.0f));
  transform.setRotation(Quaternion(Degree(-30.0f), Degree(-60.0f), Degree(0.0f)));

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

  // Moving Point Lights - Create more lights with varied Y positions for better floor coverage
  std::vector<std::pair<Vector3, Colour>> lightConfigs = {
      {Vector3(95.0f, 20.0f, 0.0f), Colour(255, 80, 80)},     // Red - higher start
      {Vector3(-51.0f, 15.0f, 0.0f), Colour(80, 255, 80)},    // Green
      {Vector3(12.0f, 35.0f, 0.0f), Colour(80, 80, 255)},     // Blue - much higher start
      {Vector3(40.0f, 25.0f, -30.0f), Colour(255, 255, 80)},  // Yellow
      {Vector3(-20.0f, 40.0f, 25.0f), Colour(255, 80, 255)},  // Magenta - higher start
      {Vector3(70.0f, 18.0f, 15.0f), Colour(80, 255, 255)},   // Cyan
      {Vector3(-80.0f, 45.0f, -10.0f), Colour(255, 150, 80)}, // Orange - highest start
      {Vector3(0.0f, 30.0f, 0.0f), Colour(150, 255, 150)},    // Light Green
      {Vector3(25.0f, 12.0f, 20.0f), Colour(200, 200, 255)},  // Light Blue - lower start
      {Vector3(-30.0f, 50.0f, -15.0f), Colour(255, 200, 150)} // Warm White - very high start
  };

  for (size_t i = 0; i < lightConfigs.size(); ++i)
  {
    std::string lightName = "MovingPointLight" + std::to_string(i + 1);
    GameObject &pointLight = _scene.createGameObject(lightName);
    auto &transform = pointLight.getComponent<TransformComponent>();
    auto &light = pointLight.addComponent<LightComponent>();

    light.setLightType(LightComponentType::Point)
        .setColour(lightConfigs[i].second)
        .setRadius(60.0f)       // Reduced radius for more focused lighting
        .setIntensity(200.0f)   // Significantly reduced intensity to prevent excessive specular highlights
        .setCastsShadows(true); // All lights now cast shadows

    Vector3 basePos = lightConfigs[i].first;
    transform.setPosition(basePos);

    // Setup for movement with varying speeds and ranges (slowed down)
    float32 speed = 0.3f + (i * 0.1f);  // Reduced from 0.8f + (i * 0.3f)
    float32 range = 30.0f + (i * 6.0f); // Slightly increased range for better effect with larger radius
    setupMovingLight(pointLight, basePos, speed, range);
  }
}

void SponzaModern::loadSponzaModel()
{
  // Load the Sponza model and integrate via ModelLoader
  GameObject &modelRoot = ModelLoader::fromFile(_scene, "./Models/sponza_pbr/sponza.obj", true);

  // IMPORTANT: Apply uniform scale to reduce model size
  // This scale factor affects the entire scene coordinate system:
  // - Camera positions must be scaled accordingly
  // - Light positions and radii must be scaled accordingly
  // - Movement factors must be adjusted for the new scale
  modelRoot.transform().setScale(Vector3(0.1f, 0.1f, 0.1f));
}

void SponzaModern::onUpdate(uint32 dtMs)
{
  if (!_cameraComponent)
    return;

  // Update moving lights
  updateMovingLights(dtMs);

  // FIX: Correct mouse delta calculation (current - last for proper direction)
  Vector2I mousePosDelta = _currentMousePos - _lastMousePos;

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

  // FIX: Invert deltaX for correct yaw direction (mouse left should turn left)
  // Calculate yaw around world up and pitch around camera right
  Radian yawRadian(-deltaX * CAMERA_LOOK_SENSITIVITY);   // Negative for correct direction
  Radian pitchRadian(-deltaY * CAMERA_LOOK_SENSITIVITY); // Negative for correct direction

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

  // FIX: Apply rotations in proper order for FPS camera
  // First apply pitch to current rotation, then apply yaw
  Quaternion newRot = yawQuat * (pitchQuat * transform.getRotation());
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
    deltaX = -CAMERA_ARROW_LOOK_SENSITIVITY * static_cast<float32>(dtMs) / 1000.0f; // Negative for left turn
  }
  else if (_inputHandler->isButtonPressed(Button::Key_Right))
  {
    deltaX = CAMERA_ARROW_LOOK_SENSITIVITY * static_cast<float32>(dtMs) / 1000.0f; // Positive for right turn
  }

  // Vertical rotation (pitch) - Up/Down arrows
  if (_inputHandler->isButtonPressed(Button::Key_Up))
  {
    deltaY = -CAMERA_ARROW_LOOK_SENSITIVITY * static_cast<float32>(dtMs) / 1000.0f; // Negative for up look
  }
  else if (_inputHandler->isButtonPressed(Button::Key_Down))
  {
    deltaY = CAMERA_ARROW_LOOK_SENSITIVITY * static_cast<float32>(dtMs) / 1000.0f; // Positive for down look
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

void SponzaModern::setupMovingLight(GameObject &lightObj, const Vector3 &basePos, float32 speed, float32 range)
{
  MovingLight movingLight;
  movingLight.gameObject = &lightObj;
  movingLight.basePosition = basePos;
  movingLight.speed = speed;
  movingLight.range = range;

  // Create a random time offset for each light to avoid synchronization
  static std::random_device rd;
  static std::mt19937 gen(rd());
  std::uniform_real_distribution<float32> offsetDist(0.0f, Math::Pi * 2.0f);
  movingLight.timeOffset = offsetDist(gen);

  // Random initial velocity direction
  std::uniform_real_distribution<float32> velDist(-1.0f, 1.0f);
  movingLight.velocity = Vector3(velDist(gen), velDist(gen) * 0.5f, velDist(gen)); // Reduced Y component
  movingLight.velocity = Vector3::Normalize(movingLight.velocity) * speed;

  _movingLights.push_back(movingLight);
}

void SponzaModern::updateMovingLights(uint32 dtMs)
{
  _totalTime += static_cast<float32>(dtMs) / 1000.0f;

  for (auto &movingLight : _movingLights)
  {
    if (!movingLight.gameObject)
      continue;

    auto &transform = movingLight.gameObject->getComponent<TransformComponent>();

    // Create smooth, organic movement using multiple sine waves with different phases (slowed down)
    float32 time = _totalTime * movingLight.speed + movingLight.timeOffset;

    // Primary orbital motion around the base position (reduced frequencies)
    Vector3 offset;
    offset.X = std::sin(time * 0.4f) * movingLight.range * 0.6f + std::sin(time * 0.8f) * movingLight.range * 0.3f;
    offset.Z = std::cos(time * 0.3f) * movingLight.range * 0.8f + std::cos(time * 1.0f) * movingLight.range * 0.2f;

    // Enhanced vertical movement to reach second floor (larger range and amplitude)
    offset.Y = std::sin(time * 0.7f) * 25.0f + std::cos(time * 1.2f) * 15.0f + std::sin(time * 0.4f) * 10.0f;

    // Add some figure-8 patterns for more complex movement (slower)
    float32 figure8Factor = std::sin(time * 0.2f);
    offset.X += std::sin(time * 1.2f) * figure8Factor * movingLight.range * 0.2f;
    offset.Z += std::cos(time * 2.4f) * figure8Factor * movingLight.range * 0.15f;

    // Apply the movement
    Vector3 newPosition = movingLight.basePosition + offset;

    // Keep lights within reasonable bounds, allowing them to reach second floor
    newPosition.Y = Math::Clamp(newPosition.Y, 5.0f, 80.0f);

    transform.setPosition(newPosition);

    // Optional: Slightly vary the light intensity for more dynamic effect (slower variation)
    if (auto *lightComponent = movingLight.gameObject->tryGetComponent<LightComponent>())
    {
      float32 intensityVariation = 1.0f + std::sin(time * 1.8f) * 0.15f; // Reduced from 3.0f to 1.8f
      lightComponent->setIntensity(1000.0f * intensityVariation);        // Updated to match new base intensity
    }
  }
}
