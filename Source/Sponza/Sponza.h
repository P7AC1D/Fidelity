#pragma once
#include <memory>
#include <vector>

#include "../Engine/Core/Fidelity.h"
#include "../Engine/Core/GameObject.h"

class CameraComponent;
class LightComponent;

struct MovingLight
{
  GameObject *gameObject = nullptr;
  Vector3 basePosition;
  Vector3 velocity;
  float32 timeOffset;
  float32 speed;
  float32 range;
};

class SponzaModern : public Application
{
public:
  SponzaModern(const ApplicationDesc &desc);

  void onStart() override;
  void onUpdate(uint32 dtMs) override;

private:
  // Camera references for easy access
  CameraComponent *_cameraComponent = nullptr;

  // Moving lights system
  std::vector<MovingLight> _movingLights;
  float32 _totalTime = 0.0f;

  // Helper methods
  void createCamera();
  void createLights();
  void loadSponzaModel();

  // Moving lights system
  void updateMovingLights(uint32 dtMs);
  void setupMovingLight(GameObject &lightObj, const Vector3 &basePos, float32 speed = 1.0f, float32 range = 30.0f);

  // Camera movement (modernized)
  void translateCamera(float32 deltaX, float32 deltaY);
  void fpsCameraLook(float32 deltaX, float32 deltaY, uint32 dtMs);
  void handleArrowKeyLook(uint32 dtMs); // New arrow key look handler

  // Helper function to extract pitch from quaternion for clamping
  float32 extractPitchFromQuaternion(const Quaternion &rotation) const;

  // Constants - Updated for better responsiveness
  static constexpr float32 CAMERA_MOVE_FACTOR = 0.1f;
  static constexpr float32 CAMERA_MOVE_SPRINT_FACTOR = 0.5f;
  static constexpr float32 CAMERA_LOOK_SENSITIVITY = 0.05f;
  static constexpr float32 CAMERA_ARROW_LOOK_SENSITIVITY = 25.0f; // Arrow key sensitivity (higher for keyboard input)
  static constexpr float32 CAMERA_MAX_PITCH_DEGREES = 89.0f;      // Prevent gimbal lock
};
