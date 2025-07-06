#include "catch.hpp"
#include <chrono>

#include "../Engine/Core/ComponentManager.h"
#include "../Engine/Core/GameObject.h"
#include "../Engine/Core/TransformComponent.h"
#include "../Engine/Rendering/DrawableComponent.h"
#include "../Engine/Rendering/CameraComponent.h"

TEST_CASE("PHASE_3_AUTOMATIC_DEPENDENCY_RESOLUTION")
{
    SECTION("AUTOMATIC_DRAWABLE_TRANSFORM_DEPENDENCY")
    {
        ComponentManager manager;
        GameObject gameObject("AutoDepTest", 1, &manager);
        
        // Add TransformComponent first
        auto& transform = gameObject.addComponent<TransformComponent>();
        transform.setPosition(Vector3(5.0f, 10.0f, 15.0f));
        
        // Add DrawableComponent - dependencies should be resolved automatically
        auto& drawable = gameObject.addComponent<DrawableComponent>();
        
        // Verify dependency was resolved automatically
        Vector3 worldPos = drawable.getWorldPosition();
        REQUIRE(worldPos == Vector3(5.0f, 10.0f, 15.0f));
        
        // Verify the dependency is actually connected
        auto transformDep = drawable.getTransformComponent();
        REQUIRE_FALSE(transformDep.expired());
    }
    
    SECTION("AUTOMATIC_CAMERA_TRANSFORM_DEPENDENCY")
    {
        ComponentManager manager;
        GameObject gameObject("CameraDepTest", 1, &manager);
        
        // Add TransformComponent first
        auto& transform = gameObject.addComponent<TransformComponent>();
        transform.setPosition(Vector3(0.0f, 5.0f, 10.0f));
        
        // Add CameraComponent - dependencies should be resolved automatically
        auto& camera = gameObject.addComponent<CameraComponent>();
        
        // Verify dependency was resolved automatically
        Vector3 worldPos = camera.getWorldPosition();
        REQUIRE(worldPos == Vector3(0.0f, 5.0f, 10.0f));
        
        // Verify the dependency is actually connected
        auto transformDep = camera.getTransformComponent();
        REQUIRE_FALSE(transformDep.expired());
        
        // Test camera functionality with automatic transform integration
        Vector3 forward = camera.getWorldForward();
        REQUIRE(forward.Length() > 0.9f); // Should be normalized
    }
    
    SECTION("DEPENDENCY_RESOLUTION_ORDER_INDEPENDENCE")
    {
        ComponentManager manager;
        GameObject gameObject("OrderTest", 1, &manager);
        
        // Add DrawableComponent BEFORE TransformComponent
        auto& drawable = gameObject.addComponent<DrawableComponent>();
        
        // Initially, drawable should have no transform dependency
        auto transformDep = drawable.getTransformComponent();
        REQUIRE(transformDep.expired());
        
        // Now add TransformComponent
        auto& transform = gameObject.addComponent<TransformComponent>();
        transform.setPosition(Vector3(20.0f, 30.0f, 40.0f));
        
        // Dependencies should be resolved when TransformComponent is added
        Vector3 worldPos = drawable.getWorldPosition();
        REQUIRE(worldPos == Vector3(20.0f, 30.0f, 40.0f));
        
        // Verify dependency is now connected
        transformDep = drawable.getTransformComponent();
        REQUIRE_FALSE(transformDep.expired());
    }
    
    SECTION("MULTIPLE_COMPONENTS_SAME_DEPENDENCY")
    {
        ComponentManager manager;
        GameObject gameObject("MultiDepTest", 1, &manager);
        
        // Add TransformComponent
        auto& transform = gameObject.addComponent<TransformComponent>();
        transform.setPosition(Vector3(1.0f, 2.0f, 3.0f));
        
        // Add multiple components that depend on TransformComponent
        auto& drawable = gameObject.addComponent<DrawableComponent>();
        auto& camera = gameObject.addComponent<CameraComponent>();
        
        // Both should have their dependencies resolved
        REQUIRE(drawable.getWorldPosition() == Vector3(1.0f, 2.0f, 3.0f));
        REQUIRE(camera.getWorldPosition() == Vector3(1.0f, 2.0f, 3.0f));
        
        // Both should have valid transform dependencies
        REQUIRE_FALSE(drawable.getTransformComponent().expired());
        REQUIRE_FALSE(camera.getTransformComponent().expired());
    }
    
    SECTION("DEPENDENCY_UPDATES_WHEN_TRANSFORM_CHANGES")
    {
        ComponentManager manager;
        GameObject gameObject("UpdateTest", 1, &manager);
        
        auto& transform = gameObject.addComponent<TransformComponent>();
        auto& drawable = gameObject.addComponent<DrawableComponent>();
        auto& camera = gameObject.addComponent<CameraComponent>();
        
        // Initial position
        transform.setPosition(Vector3(10.0f, 20.0f, 30.0f));
        REQUIRE(drawable.getWorldPosition() == Vector3(10.0f, 20.0f, 30.0f));
        REQUIRE(camera.getWorldPosition() == Vector3(10.0f, 20.0f, 30.0f));
        
        // Change position
        transform.setPosition(Vector3(100.0f, 200.0f, 300.0f));
        REQUIRE(drawable.getWorldPosition() == Vector3(100.0f, 200.0f, 300.0f));
        REQUIRE(camera.getWorldPosition() == Vector3(100.0f, 200.0f, 300.0f));
    }
    
    SECTION("CAMERA_SPECIFIC_FUNCTIONALITY")
    {
        ComponentManager manager;
        GameObject gameObject("CameraTest", 1, &manager);
        
        auto& transform = gameObject.addComponent<TransformComponent>();
        auto& camera = gameObject.addComponent<CameraComponent>();
        
        // Configure camera
        camera.setPerspective(Degree(60.0f), 1920, 1080, 0.1f, 1000.0f);
        
        // Test camera properties
        REQUIRE(camera.getWidth() == 1920);
        REQUIRE(camera.getHeight() == 1080);
        REQUIRE(camera.getAspectRatio() == (1920.0f / 1080.0f));
        
        // Test view matrix calculation (should not crash)
        Matrix4 view = camera.getView();
        REQUIRE(view != Matrix4::Identity);
        
        // Test projection matrix
        Matrix4 proj = camera.getProj();
        REQUIRE(proj != Matrix4::Identity);
        
        // Test world space vectors
        Vector3 forward = camera.getWorldForward();
        Vector3 up = camera.getWorldUp();
        Vector3 right = camera.getWorldRight();
        
        // Vectors should be normalized
        REQUIRE(std::abs(forward.Length() - 1.0f) < 0.01f);
        REQUIRE(std::abs(up.Length() - 1.0f) < 0.01f);
        REQUIRE(std::abs(right.Length() - 1.0f) < 0.01f);
    }
    
    SECTION("PERFORMANCE_WITH_AUTOMATIC_DEPENDENCIES")
    {
        ComponentManager manager;
        std::vector<std::unique_ptr<GameObject>> gameObjects;
        
        const int objectCount = 50;
        
        // Time creation of many objects with automatic dependency resolution
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < objectCount; ++i)
        {
            auto gameObject = std::make_unique<GameObject>("PerfObject" + std::to_string(i), i, &manager);
            
            // Add components in various orders to test dependency resolution
            if (i % 2 == 0)
            {
                // Transform first
                gameObject->addComponent<TransformComponent>();
                gameObject->addComponent<DrawableComponent>();
                gameObject->addComponent<CameraComponent>();
            }
            else
            {
                // Components first, then transform
                gameObject->addComponent<DrawableComponent>();
                gameObject->addComponent<CameraComponent>();
                gameObject->addComponent<TransformComponent>();
            }
            
            gameObjects.push_back(std::move(gameObject));
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        // Should be fast even with automatic dependency resolution
        REQUIRE(duration.count() < 10000); // Less than 10ms
        
        // Verify all dependencies were resolved correctly
        for (auto& obj : gameObjects)
        {
            auto& drawable = obj->getComponent<DrawableComponent>();
            auto& camera = obj->getComponent<CameraComponent>();
            
            REQUIRE_FALSE(drawable.getTransformComponent().expired());
            REQUIRE_FALSE(camera.getTransformComponent().expired());
        }
    }
}

TEST_CASE("PHASE_3_COMPARISON_WITH_MANUAL_DEPENDENCY_WIRING")
{
    SECTION("AUTOMATIC_VS_MANUAL_DEPENDENCY_SETUP")
    {
        ComponentManager manager;
        
        // Manual dependency setup (old way)
        {
            GameObject manualObject("Manual", 1, &manager);
            auto& transform = manualObject.addComponent<TransformComponent>();
            auto& drawable = manualObject.addComponent<DrawableComponent>();
            
            // Would need manual wiring in old system
            auto transformPtr = manualObject.tryGetComponent<TransformComponent>();
            drawable.setTransformComponent(
                std::shared_ptr<TransformComponent>(transformPtr, [](TransformComponent*){})
            );
            
            transform.setPosition(Vector3(1.0f, 2.0f, 3.0f));
            REQUIRE(drawable.getWorldPosition() == Vector3(1.0f, 2.0f, 3.0f));
        }
        
        // Automatic dependency setup (new way)
        {
            GameObject autoObject("Auto", 2, &manager);
            auto& transform = autoObject.addComponent<TransformComponent>();
            auto& drawable = autoObject.addComponent<DrawableComponent>();
            
            // No manual wiring needed! Dependencies resolved automatically
            
            transform.setPosition(Vector3(1.0f, 2.0f, 3.0f));
            REQUIRE(drawable.getWorldPosition() == Vector3(1.0f, 2.0f, 3.0f));
        }
        
        // Both approaches work, but automatic is much cleaner
    }
}
