#pragma once
#include <memory>
#include <vector>

#include "../Engine/Core/Fidelity.h"
#include "../Engine/Core/GameObject.h"

class CameraComponent;
class LightComponent;

class SponzaModern : public Application
{
public:
  SponzaModern(const ApplicationDesc &desc);

  void onStart() override;
  void onUpdate(uint32 dtMs) override;

private:
  // Camera references for easy access
  CameraComponent *_cameraComponent = nullptr;

  // Helper methods
  void createCamera();
  void createLights();
  void loadSponzaModel();

  // Camera movement (modernized)
  void translateCamera(float32 deltaX, float32 deltaY);
  void fpsCameraLook(float32 deltaX, float32 deltaY, uint32 dtMs);
  void handleArrowKeyLook(uint32 dtMs);  // New arrow key look handler

  // Helper function to extract pitch from quaternion for clamping
  float32 extractPitchFromQuaternion(const Quaternion &rotation) const;

  // Constants - Updated for better responsiveness
  static constexpr float32 CAMERA_MOVE_FACTOR = 0.5f;
  static constexpr float32 CAMERA_MOVE_SPRINT_FACTOR = 2.0f;
  static constexpr float32 CAMERA_LOOK_SENSITIVITY = 0.1f;
  static constexpr float32 CAMERA_ARROW_LOOK_SENSITIVITY = 50.0f; // Arrow key sensitivity (higher for keyboard input)
  static constexpr float32 CAMERA_MAX_PITCH_DEGREES = 89.0f; // Prevent gimbal lock
};
