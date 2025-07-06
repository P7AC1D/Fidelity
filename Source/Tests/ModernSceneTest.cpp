#include "catch.hpp"
#include "../Engine/Core/Scene.h"
#include "../Engine/Core/TransformComponent.h"
#include "../Engine/Rendering/CameraComponent.h"
#include "../Engine/Rendering/DrawableComponent.h"
#include "../Engine/Rendering/LightComponent.h"
#include <chrono>

// Mock InputHandler for testing
class MockInputHandler
{
public:
    // Add minimal interface needed for testing
};

TEST_CASE("Modern Scene System", "[Scene][Scene]")
{
    auto mockInputHandler = std::make_shared<MockInputHandler>();
    
    SECTION("Scene Creation and Initialization")
    {
        // Create scene - this should work without a real InputHandler for basic testing
        Scene scene(nullptr); // Using nullptr for now since we're not testing input
        
        // Test basic scene operations without full initialization
        SECTION("GameObject Creation")
        {
            auto& gameObject = scene.createGameObject("TestObject");
            
            // Verify GameObject was created with basic properties
            REQUIRE(gameObject.getName() == "TestObject");
            REQUIRE(gameObject.getIndex() == 1); // First created object should have ID 1
            REQUIRE(gameObject.isActive() == true);
            
            // Verify it has a TransformComponent by default
            REQUIRE(gameObject.hasComponent<TransformComponent>());
            auto* transform = gameObject.tryGetComponent<TransformComponent>();
            REQUIRE(transform != nullptr);
        }
        
        SECTION("Component Collection")
        {
            // Create multiple GameObjects with different components
            auto& camera1 = scene.createGameObject("Camera1");
            camera1.addComponent<CameraComponent>();
            
            auto& camera2 = scene.createGameObject("Camera2");
            camera2.addComponent<CameraComponent>();
            
            auto& light1 = scene.createGameObject("Light1");
            light1.addComponent<LightComponent>();
            
            auto& drawable1 = scene.createGameObject("Drawable1");
            drawable1.addComponent<DrawableComponent>();
            
            // Test component collection
            auto cameras = scene.getCameras();
            auto lights = scene.getLights();
            auto drawables = scene.getDrawables();
            
            REQUIRE(cameras.size() == 2);
            REQUIRE(lights.size() == 1);
            REQUIRE(drawables.size() == 1);
            
            // Test main camera selection
            auto* mainCamera = scene.getMainCamera();
            REQUIRE(mainCamera != nullptr);
            REQUIRE(mainCamera == cameras[0]); // Should be the first camera
        }
        
        SECTION("Hierarchy Management")
        {
            auto& parent = scene.createGameObject("Parent");
            auto child = std::make_unique<GameObject>("Child", 100, nullptr);
            child->addComponent<TransformComponent>();
            
            // Add child to parent
            scene.addChild(parent, std::move(child));
            
            // Verify hierarchy
            REQUIRE(parent.getChildren().size() == 1);
            REQUIRE(parent.getChildren()[0]->getName() == "Child");
        }
    }
    
    SECTION("Scene Update and Management")
    {
        Scene scene(nullptr);
        
        // Create some GameObjects
        auto& obj1 = scene.createGameObject("Object1");
        auto& obj2 = scene.createGameObject("Object2");
        auto& obj3 = scene.createGameObject("Object3");
        
        // Add various components
        obj1.addComponent<CameraComponent>();
        obj2.addComponent<LightComponent>();
        obj3.addComponent<DrawableComponent>();
        
        // Test scene update (should not crash)
        REQUIRE_NOTHROW(scene.update(0.016f)); // 60 FPS delta time
        
        // Verify all objects are still valid after update
        REQUIRE(obj1.isActive());
        REQUIRE(obj2.isActive());
        REQUIRE(obj3.isActive());
        
        // Test component access after update
        REQUIRE(scene.getCameras().size() == 1);
        REQUIRE(scene.getLights().size() == 1);
        REQUIRE(scene.getDrawables().size() == 1);
    }
    
    SECTION("Component Dependencies in Scene Context")
    {
        Scene scene(nullptr);
        
        auto& gameObject = scene.createGameObject("TestObject");
        
        // Add components that depend on TransformComponent
        auto& camera = gameObject.addComponent<CameraComponent>();
        auto& light = gameObject.addComponent<LightComponent>();
        auto& drawable = gameObject.addComponent<DrawableComponent>();
        
        // Verify dependencies are resolved
        REQUIRE_FALSE(camera.getTransformComponent().expired());
        REQUIRE_FALSE(light.getTransformComponent().expired());
        REQUIRE_FALSE(drawable.getTransformComponent().expired());
        
        // Test that components can access transform data
        auto* transform = gameObject.tryGetComponent<TransformComponent>();
        REQUIRE(transform != nullptr);
        
        Vector3 testPosition(10, 20, 30);
        transform->setPosition(testPosition);
        
        // Components should reflect the transform position
        REQUIRE(camera.getWorldPosition() == testPosition);
        REQUIRE(light.getPosition() == testPosition);
    }
}

TEST_CASE("Modern Scene Performance", "[Scene][Performance]")
{
    Scene scene(nullptr);
    
    SECTION("Large Number of GameObjects")
    {
        const int numObjects = 1000;
        std::vector<GameObject*> objects;
        
        // Create many objects
        for (int i = 0; i < numObjects; ++i)
        {
            auto& obj = scene.createGameObject("Object" + std::to_string(i));
            objects.push_back(&obj);
            
            // Add components to some objects
            if (i % 10 == 0) obj.addComponent<CameraComponent>();
            if (i % 5 == 0) obj.addComponent<LightComponent>();
            if (i % 3 == 0) obj.addComponent<DrawableComponent>();
        }
        
        // Verify all objects were created
        REQUIRE(objects.size() == numObjects);
        
        // Test component collection performance
        auto cameras = scene.getCameras();
        auto lights = scene.getLights();
        auto drawables = scene.getDrawables();
        
        // Verify expected counts
        REQUIRE(cameras.size() == numObjects / 10);
        REQUIRE(lights.size() == numObjects / 5);
        REQUIRE(drawables.size() == numObjects / 3);
        
        // Test update performance (should complete quickly)
        auto startTime = std::chrono::high_resolution_clock::now();
        scene.update(0.016f);
        auto endTime = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        
        // Update should complete in reasonable time (less than 100ms for 1000 objects)
        REQUIRE(duration.count() < 100);
    }
}
