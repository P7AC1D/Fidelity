#include "Test3D.h"

#include <iomanip>
#include <random>
#include <sstream>

Test3D::Test3D(const ApplicationDesc &desc) 
    : Application(desc)
    , _scene(_inputHandler)
    , _camera(nullptr)
    , _cameraComponent(nullptr)
{
}

void Test3D::onStart()
{
    // Initialize the modern scene - for now, pass nullptr for render device
    // The scene will handle renderer creation internally
    if (!_scene.init(Vector2I(getWidth(), getHeight()), nullptr))
    {
        // Handle initialization failure
        return;
    }

    // Create main camera
    auto& cameraObj = _scene.createGameObject("mainCamera");
    _camera = &cameraObj;
    _cameraComponent = &cameraObj.addComponent<CameraComponent>();
    
    // Set up camera properties (same as legacy)
    _cameraComponent->setPerspective(Degree(67.67f), getWidth(), getHeight(), 0.1f, 200.0f);
    
    // Set camera transform
    auto* cameraTransform = cameraObj.tryGetComponent<TransformComponent>();
    if (cameraTransform)
    {
        cameraTransform->setPosition(Vector3(-15.0f, 17.0f, -11.0f));
        cameraTransform->setRotation(Quaternion(Degree(-123.0f), Degree(36.0f), Degree(138.0f)));
    }

    // Create directional light
    auto& dirLightObj = _scene.createGameObject("directionalLight");
    auto& dirLight = dirLightObj.addComponent<LightComponent>();
    dirLight.setLightType(LightComponentType::Directional)
           .setColour(Colour(244, 233, 155))
           .setIntensity(1.0f);
    
    auto* dirLightTransform = dirLightObj.tryGetComponent<TransformComponent>();
    if (dirLightTransform)
    {
        dirLightTransform->setRotation(Quaternion(Degree(-23), Degree(-73.f), Degree(26.0f)));
    }

    // Create point lights
    auto& light1Obj = _scene.createGameObject("light1");
    auto& light1 = light1Obj.addComponent<LightComponent>();
    light1.setLightType(LightComponentType::Point)
          .setColour(Colour(150, 25, 25))
          .setRadius(25.0f);
    
    auto* light1Transform = light1Obj.tryGetComponent<TransformComponent>();
    if (light1Transform)
    {
        light1Transform->setPosition(Vector3(0.0f, 1.0f, 0.0f));
    }

    auto& light2Obj = _scene.createGameObject("light2");
    auto& light2 = light2Obj.addComponent<LightComponent>();
    light2.setLightType(LightComponentType::Point)
          .setColour(Colour(25, 150, 25))
          .setRadius(25.0f);
    
    auto* light2Transform = light2Obj.tryGetComponent<TransformComponent>();
    if (light2Transform)
    {
        light2Transform->setPosition(Vector3(-1.5f, 1.0f, -1.0f));
    }

    auto& light3Obj = _scene.createGameObject("light3");
    auto& light3 = light3Obj.addComponent<LightComponent>();
    light3.setLightType(LightComponentType::Point)
          .setColour(Colour(25, 25, 100))
          .setRadius(25.0f);
    
    auto* light3Transform = light3Obj.tryGetComponent<TransformComponent>();
    if (light3Transform)
    {
        light3Transform->setPosition(Vector3(-1.5f, 1.0f, 1.0f));
    }

    // Create material for cubes
    std::shared_ptr<Material> material(new Material());
    material->setDiffuseTexture(loadTextureFromFile("./Textures/crate0_diffuse.png", true, true));
    material->setNormalTexture(loadTextureFromFile("./Textures/crate0_normal.png", false, false));
    material->setMetallicTexture(loadTextureFromFile("./Textures/crate0_bump.png", false, false));

    // Create random cubes (same as legacy)
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dist(-25, 25);
    
    for (uint32 i = 0; i < 100; i++)
    {
        auto& cubeObj = _scene.createGameObject("cube" + std::to_string(i));
        auto& drawable = cubeObj.addComponent<DrawableComponent>();
        
        // Set up drawable (mesh will be set when mesh system is modernized)
        drawable.setMaterial(material);
        drawable.setVisible(true);
        
        // Set transform
        auto* cubeTransform = cubeObj.tryGetComponent<TransformComponent>();
        if (cubeTransform)
        {
            cubeTransform->setPosition(Vector3(static_cast<float32>(dist(gen)), std::fabsf(static_cast<float32>(dist(gen))), static_cast<float32>(dist(gen))));
            cubeTransform->setRotation(Quaternion(Degree(static_cast<float32>(dist(gen))), Degree(static_cast<float32>(dist(gen))), Degree(static_cast<float32>(dist(gen)))));
        }
    }

    // Create floor
    std::shared_ptr<Material> floorMaterial(new Material());
    floorMaterial->setDiffuseTexture(loadTextureFromFile("./Textures/brick_floor_tileable_Base_Color.jpg", true, true));
    
    auto& floorObj = _scene.createGameObject("floor");
    auto& floorDrawable = floorObj.addComponent<DrawableComponent>();
    floorDrawable.setMaterial(floorMaterial);
    floorDrawable.setVisible(true);
    
    auto* floorTransform = floorObj.tryGetComponent<TransformComponent>();
    if (floorTransform)
    {
        floorTransform->setPosition(Vector3(50.0f, -5.0f, 50.0f));
        floorTransform->setScale(Vector3(100.0f, 100.0f, 100.0f));
    }
}

void Test3D::onUpdate(uint32 dtMs)
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

    // Render the scene using modern components with legacy renderer bridge
    _scene.drawFrame();
}

void Test3D::translateCamera(float32 deltaX, float32 deltaY)
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

void Test3D::fpsCameraLook(int32 mouseDeltaX, int32 mouseDeltaY, uint32 dtMs)
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
