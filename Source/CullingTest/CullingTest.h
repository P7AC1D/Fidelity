#pragma once
#include <memory>

#include "../Engine/Core/Fidelity.h"
#include "../Engine/Core/Scene.h"
#include "../Engine/Core/GameObject.h"
#include "../Engine/Core/TransformComponent.h"
#include "../Engine/Rendering/CameraComponent.h"
#include "../Engine/Rendering/LightComponent.h"
#include "../Engine/Rendering/DrawableComponent.h"
#include "../Engine/Rendering/Material.h"

class CullingTest : public Application
{
public:
    CullingTest(const ApplicationDesc &desc);

    void onStart() override;
    void onUpdate(uint32 dtMs) override;

private:
    GameObject* _camera;
    CameraComponent* _cameraComponent;
    
    // Performance tracking for culling tests
    uint32 _totalObjects;
    uint32 _visibleObjects;
    uint32 _culledObjects;
    
    // Culling performance analysis
    void updateCullingStats();
    void drawCullingDebugInfo();
};
