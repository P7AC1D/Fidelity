#pragma once
#include <memory>

#include "../Engine/Core/Fidelity.h"
#include "../Engine/Core/Scene.h"
#include "../Engine/Core/GameObject.h"
#include "../Engine/Rendering/CameraComponent.h"
#include "../Engine/Rendering/LightComponent.h"
#include "../Engine/Rendering/DrawableComponent.h"

class CullingTest : public Application
{
public:
    CullingTest(const ApplicationDesc &desc);

    void onStart() override;
    void onUpdate(uint32 dtMs) override;

private:
    Scene _scene;
    GameObject* _camera;
    CameraComponent* _cameraComponent;
    
    // Performance tracking for culling tests
    uint32 _totalObjects;
    uint32 _visibleObjects;
    uint32 _culledObjects;
    
    // Helper methods for camera movement (same as legacy)
    void translateCamera(float32 deltaX, float32 deltaY);
    void fpsCameraLook(int32 mouseDeltaX, int32 mouseDeltaY, uint32 dtMs);
    
    // Culling performance analysis
    void updateCullingStats();
    void drawCullingDebugInfo();
    
    // Camera movement constants
    static constexpr float32 CAMERA_MOVE_FACTOR = 0.01f;
    static constexpr float32 CAMERA_MOVE_SPRINT_FACTOR = 0.05f;
    static constexpr float32 CAMERA_LOOK_SENSITIVITY = 0.1f;
};
