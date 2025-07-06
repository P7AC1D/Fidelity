#include "catch.hpp"
#include "../Engine/Core/GameObject.h"
#include "../Engine/Core/ComponentManager.h"
#include "../Engine/Core/TransformComponent.h"
#include "../Engine/Rendering/CameraComponent.h"
#include "../Engine/Rendering/DrawableComponent.h"
#include "../Engine/Rendering/LightComponent.h"

TEST_CASE("Modern Component System Integration", "[ModernComponents][Integration]")
{
    ComponentManager componentManager;

    SECTION("Complete Scene Setup with All Modern Components")
    {
        // Create a camera object
        auto camera = std::make_unique<GameObject>("MainCamera", 1, &componentManager);
        auto& cameraTransform = camera->addComponent<TransformComponent>();
        auto& cameraComponent = camera->addComponent<CameraComponent>();
        
        // Configure camera
        cameraTransform.setPosition(Vector3(0, 5, 10));
        cameraTransform.setRotation(Quaternion(Degree(0), Degree(-15), Degree(0)));
        cameraComponent.setPerspective(Degree(60), 1920, 1080, 0.1f, 1000.0f);
        
        // Verify camera dependency injection worked
        REQUIRE_FALSE(cameraComponent.getTransformComponent().expired());
        REQUIRE(cameraComponent.getWorldPosition() == Vector3(0, 5, 10));
        
        // Create a light object
        auto light = std::make_unique<GameObject>("MainLight", 2, &componentManager);
        auto& lightTransform = light->addComponent<TransformComponent>();
        auto& lightComponent = light->addComponent<LightComponent>();
        
        // Configure light
        lightTransform.setPosition(Vector3(5, 10, 5));
        lightComponent.setLightType(LightComponentType::Point)
                     .setColour(Colour(255, 255, 255))
                     .setRadius(50.0f)
                     .setIntensity(1000.0f)
                     .setCastsShadows(true);
        
        // Verify light dependency injection worked
        REQUIRE_FALSE(lightComponent.getTransformComponent().expired());
        REQUIRE(lightComponent.getPosition() == Vector3(5, 10, 5));
        
        // Create a drawable object
        auto drawable = std::make_unique<GameObject>("TestObject", 3, &componentManager);
        auto& drawableTransform = drawable->addComponent<TransformComponent>();
        auto& drawableComponent = drawable->addComponent<DrawableComponent>();
        
        // Configure drawable
        drawableTransform.setPosition(Vector3(0, 0, 0));
        drawableTransform.setScale(Vector3(2, 2, 2));
        
        // Verify drawable dependency injection worked
        REQUIRE_FALSE(drawableComponent.getTransformComponent().expired());
        
        // Test component interactions
        SECTION("Camera Frustum Culling")
        {
            // Test if drawable is visible from camera
            auto drawableMatrix = drawableTransform.getWorldMatrix();
            Aabb testBounds(Vector3(-1, -1, -1), Vector3(1, 1, 1));
            
            // The drawable should be visible from this camera position
            bool isVisible = cameraComponent.contains(testBounds, drawableMatrix);
            REQUIRE(isVisible);
        }
        
        SECTION("Light Position Tracking")
        {
            // Test light position tracking
            Vector3 lightPos = lightComponent.getPosition();
            Vector3 drawablePos = drawableTransform.getPosition();
            float expectedDistance = (Vector3(5, 10, 5) - Vector3(0, 0, 0)).Length();
            float actualDistance = (lightPos - drawablePos).Length();
            REQUIRE(actualDistance == Approx(expectedDistance).epsilon(0.01f));
        }
        
        SECTION("Transform Hierarchy")
        {
            // Test transform updates propagate to components
            Vector3 newPosition(10, 20, 30);
            lightTransform.setPosition(newPosition);
            
            // Light component should reflect the new position
            REQUIRE(lightComponent.getPosition() == newPosition);
        }
    }
    
    SECTION("Component Lifecycle Management")
    {
        auto gameObject = std::make_unique<GameObject>("TestObject", 4, &componentManager);
        
        // Add components in different orders to test dependency resolution
        auto& drawable = gameObject->addComponent<DrawableComponent>();
        auto& light = gameObject->addComponent<LightComponent>();
        auto& transform = gameObject->addComponent<TransformComponent>();
        auto& camera = gameObject->addComponent<CameraComponent>();
        
        // All components should have their dependencies resolved
        REQUIRE_FALSE(drawable.getTransformComponent().expired());
        REQUIRE_FALSE(light.getTransformComponent().expired());
        REQUIRE_FALSE(camera.getTransformComponent().expired());
        
        // Test component retrieval
        REQUIRE(gameObject->hasComponent<TransformComponent>());
        REQUIRE(gameObject->hasComponent<CameraComponent>());
        REQUIRE(gameObject->hasComponent<DrawableComponent>());
        REQUIRE(gameObject->hasComponent<LightComponent>());
        
        // Test component access
        auto* retrievedTransform = gameObject->tryGetComponent<TransformComponent>();
        REQUIRE(retrievedTransform != nullptr);
        REQUIRE(retrievedTransform == &transform);
    }
    
    SECTION("Performance and Memory Management")
    {
        std::vector<std::unique_ptr<GameObject>> objects;
        
        // Create many objects to test performance
        for (int i = 0; i < 100; ++i)
        {
            auto obj = std::make_unique<GameObject>("Object" + std::to_string(i), i + 100, &componentManager);
            obj->addComponent<TransformComponent>();
            obj->addComponent<CameraComponent>();
            objects.push_back(std::move(obj));
        }
        
        // Verify all objects are properly constructed
        REQUIRE(objects.size() == 100);
        
        // Verify all have their dependencies resolved
        for (const auto& obj : objects)
        {
            auto* camera = obj->tryGetComponent<CameraComponent>();
            REQUIRE(camera != nullptr);
            REQUIRE_FALSE(camera->getTransformComponent().expired());
        }
        
        // Objects will be automatically destroyed when vector goes out of scope
        // This tests proper cleanup and destructor chains
    }
}

TEST_CASE("Modern Component Basic Functionality", "[ModernComponents][Basic]")
{
    ComponentManager componentManager;
    
    SECTION("Modern Components Basic Operations")
    {
        // Create modern camera
        auto modernCamera = std::make_unique<GameObject>("ModernCamera", 1, &componentManager);
        auto& modernTransform = modernCamera->addComponent<TransformComponent>();
        auto& modernCameraComp = modernCamera->addComponent<CameraComponent>();
        
        // Configure camera with basic settings
        Vector3 position(10, 5, 15);
        modernTransform.setPosition(position);
        modernCameraComp.setPerspective(Degree(60), 1920, 1080, 0.1f, 1000.0f);
        
        // Test basic functionality
        REQUIRE(modernCameraComp.getWidth() == 1920);
        REQUIRE(modernCameraComp.getHeight() == 1080);
        REQUIRE(modernCameraComp.getNear() == 0.1f);
        REQUIRE(modernCameraComp.getFar() == 1000.0f);
        REQUIRE(modernCameraComp.getWorldPosition() == position);
    }
}

TEST_CASE("Modern Component Inspector Integration", "[ModernComponents][UI]")
{
    ComponentManager componentManager;
    
    SECTION("All Components Have Inspector Support")
    {
        auto gameObject = std::make_unique<GameObject>("TestObject", 1, &componentManager);
        
        auto& transform = gameObject->addComponent<TransformComponent>();
        auto& camera = gameObject->addComponent<CameraComponent>();
        auto& drawable = gameObject->addComponent<DrawableComponent>();
        auto& light = gameObject->addComponent<LightComponent>();
        
        // These calls should not crash (we can't easily test ImGui rendering in unit tests)
        // But we can verify the methods exist and are callable
        REQUIRE_NOTHROW(transform.drawInspector());
        REQUIRE_NOTHROW(camera.drawInspector());
        REQUIRE_NOTHROW(drawable.drawInspector());
        REQUIRE_NOTHROW(light.drawInspector());
    }
}
