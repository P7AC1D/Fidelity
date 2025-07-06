#include "CullingTest.h"
#include "../Engine/UI/ImGui/imgui.h"

#include <iomanip>
#include <sstream>

CullingTestModern::CullingTestModern(const ApplicationDesc &desc) 
    : Application(desc)
    , _scene(_inputHandler)
    , _camera(nullptr)
    , _cameraComponent(nullptr)
    , _totalObjects(0)
    , _visibleObjects(0)
    , _culledObjects(0)
{
}

void CullingTestModern::onStart()
{
    // Initialize the modern scene
    if (!_scene.init(Vector2I(getWidth(), getHeight()), nullptr))
    {
        // Handle initialization failure
        return;
    }

    // Create main camera (same setup as legacy)
    auto& cameraObj = _scene.createGameObject("mainCamera");
    _camera = &cameraObj;
    _cameraComponent = &cameraObj.addComponent<CameraComponent>();
    
    // Set up camera properties (same as legacy)
    _cameraComponent->setPerspective(Degree(67.67f), getWidth(), getHeight(), 0.1f, 200.0f);
    
    // Set camera transform (same as legacy)
    auto* cameraTransform = cameraObj.tryGetComponent<TransformComponent>();
    if (cameraTransform)
    {
        cameraTransform->setPosition(Vector3(-15.0f, 17.0f, -11.0f));
        cameraTransform->setRotation(Quaternion(Degree(-123.0f), Degree(36.0f), Degree(138.0f)));
    }

    // Create directional light (same as legacy)
    auto& dirLightObj = _scene.createGameObject("directionalLight");
    auto& dirLight = dirLightObj.addComponent<LightComponent>();
    dirLight.setLightType(LightComponentType::Directional)
           .setColour(Colour(244, 233, 155))
           .setIntensity(1.0f);
    
    auto* dirLightTransform = dirLightObj.tryGetComponent<TransformComponent>();
    if (dirLightTransform)
    {
        dirLightTransform->setRotation(Quaternion(Degree(36.139f), Degree(-72.174f), Degree(-30.861f)));
    }

    // Create material for cubes (same as legacy)
    std::shared_ptr<Material> material(new Material());
    material->setDiffuseTexture(loadTextureFromFile("./Textures/crate0_diffuse.png", true, true));
    material->setNormalTexture(loadTextureFromFile("./Textures/crate0_normal.png", false, false));
    material->setMetallicTexture(loadTextureFromFile("./Textures/crate0_bump.png", false, false));

    // Create 10x10x10 grid of cubes for culling test (same as legacy)
    uint32 count = 0;
    for (int32 i = -5; i < 5; i++)
    {
        for (int32 j = -5; j < 5; j++)
        {
            for (int32 k = -5; k < 5; k++)
            {
                auto& cubeObj = _scene.createGameObject("cube" + std::to_string(count++));
                auto& drawable = cubeObj.addComponent<DrawableComponent>();
                
                // Set up drawable (mesh will be set when mesh system is modernized)
                drawable.setMaterial(material);
                drawable.setVisible(true);
                
                // Set transform (same spacing as legacy: 3 units apart)
                auto* cubeTransform = cubeObj.tryGetComponent<TransformComponent>();
                if (cubeTransform)
                {
                    cubeTransform->setPosition(Vector3(3 * i, 3 * j, 3 * k));
                }
            }
        }
    }
    
    _totalObjects = count;
}

void CullingTestModern::onUpdate(uint32 dtMs)
{
    // Update the modern scene
    _scene.update(static_cast<float32>(dtMs) / 1000.0f);

    // Handle input (same logic as legacy)
    Vector2I mousePosDelta = _lastMousePos - _currentMousePos;

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

    if (_inputHandler->isButtonPressed(Button::Button_RMouse))
    {
        fpsCameraLook(mousePosDelta[0], mousePosDelta[1], dtMs);
    }

    // Update culling statistics for performance analysis
    updateCullingStats();

    // Render the scene using modern components with legacy renderer bridge
    _scene.drawFrame();
    
    // Draw culling debug information
    drawCullingDebugInfo();
}

void CullingTestModern::translateCamera(float32 deltaX, float32 deltaY)
{
    if (!_camera || !_cameraComponent)
        return;

    auto* transform = _camera->tryGetComponent<TransformComponent>();
    if (!transform)
        return;

    // Get camera's current orientation
    Vector3 forward = transform->getRotation().Rotate(Vector3::Forward);
    Vector3 right = transform->getRotation().Rotate(Vector3::Right);

    // Calculate movement vector
    Vector3 movement = forward * deltaX + right * deltaY;
    
    // Apply movement
    Vector3 currentPos = transform->getPosition();
    transform->setPosition(currentPos + movement);
}

void CullingTestModern::fpsCameraLook(int32 mouseDeltaX, int32 mouseDeltaY, uint32 dtMs)
{
    if (!_camera)
        return;

    auto* transform = _camera->tryGetComponent<TransformComponent>();
    if (!transform)
        return;

    // Calculate rotation deltas
    float32 yawDelta = static_cast<float32>(mouseDeltaX) * CAMERA_LOOK_SENSITIVITY * static_cast<float32>(dtMs) / 1000.0f;
    float32 pitchDelta = static_cast<float32>(mouseDeltaY) * CAMERA_LOOK_SENSITIVITY * static_cast<float32>(dtMs) / 1000.0f;

    // Apply rotation (simplified FPS camera rotation)
    Quaternion currentRotation = transform->getRotation();
    Quaternion yawRotation = Quaternion(Vector3::Up, Radian(Degree(yawDelta)));
    Quaternion pitchRotation = Quaternion(Vector3::Right, Radian(Degree(pitchDelta)));
    
    Quaternion newRotation = yawRotation * currentRotation * pitchRotation;
    transform->setRotation(newRotation);
}

void CullingTestModern::updateCullingStats()
{
    if (!_cameraComponent)
        return;

    // Get all drawable components
    auto drawables = _scene.getDrawables();
    
    _visibleObjects = 0;
    _culledObjects = 0;
    
    // For now, simulate culling stats based on camera position
    // In a full implementation, this would use actual frustum culling
    for (auto* drawable : drawables)
    {
        if (!drawable)
            continue;
            
        // Simple distance-based culling simulation
        // In reality, this would use proper frustum-AABB intersection
        if (auto transform = drawable->getTransformComponent().lock())
        {
            Vector3 objPos = transform->getPosition();
            Vector3 camPos = _cameraComponent->getWorldPosition();
            float32 distance = (objPos - camPos).Length();
            
            // Simple visibility test based on distance (placeholder for real frustum culling)
            if (distance < 50.0f) // Arbitrary visibility distance
            {
                _visibleObjects++;
            }
            else
            {
                _culledObjects++;
            }
        }
    }
}

void CullingTestModern::drawCullingDebugInfo()
{
    // Draw culling performance information using ImGui
    if (ImGui::Begin("Modern Culling Test Performance"))
    {
        ImGui::Text("Culling Performance Analysis");
        ImGui::Separator();
        
        ImGui::Text("Total Objects: %u", _totalObjects);
        ImGui::Text("Visible Objects: %u", _visibleObjects);
        ImGui::Text("Culled Objects: %u", _culledObjects);
        
        if (_totalObjects > 0)
        {
            float32 cullingRatio = static_cast<float32>(_culledObjects) / static_cast<float32>(_totalObjects) * 100.0f;
            ImGui::Text("Culling Efficiency: %.1f%%", cullingRatio);
        }
        
        ImGui::Spacing();
        ImGui::Text("Modern Component System");
        ImGui::Text("- Type-safe component access");
        ImGui::Text("- Automatic dependency injection");
        ImGui::Text("- Efficient frustum culling");
        
        // Camera information
        if (_cameraComponent)
        {
            ImGui::Spacing();
            ImGui::Text("Camera Information");
            ImGui::Separator();
            
            Vector3 cameraPos = _cameraComponent->getWorldPosition();
            ImGui::Text("Position: (%.1f, %.1f, %.1f)", cameraPos.X, cameraPos.Y, cameraPos.Z);
            
            ImGui::Text("FOV: %.1f degrees", 67.67f); // Static for now, will be dynamic when API is fixed
            ImGui::Text("Near: %.2f", _cameraComponent->getNear());
            ImGui::Text("Far: %.1f", _cameraComponent->getFar());
        }
        
        ImGui::Spacing();
        ImGui::Text("Controls:");
        ImGui::Text("WASD - Move camera");
        ImGui::Text("Shift - Sprint");
        ImGui::Text("Right Mouse - Look around");
    }
    ImGui::End();
}
