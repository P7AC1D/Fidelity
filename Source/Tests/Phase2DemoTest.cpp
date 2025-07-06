#include "catch.hpp"
#include <chrono>

#include "../Engine/Core/ComponentManager.h"
#include "../Engine/Core/GameObject.h"
#include "../Engine/Core/TransformComponent.h"
#include "../Engine/Rendering/DrawableComponent.h"

TEST_CASE("PHASE_2_DEMONSTRATION")
{
    SECTION("COMPLETE_RENDERABLE_GAMEOBJECT_CREATION")
    {
        ComponentManager manager;
        
        // Create a GameObject with both Transform and Drawable components
        auto gameObject = std::make_unique<GameObject>("DemoObject", 1, &manager);
        
        // Add core components
        auto& transform = gameObject->addComponent<TransformComponent>();
        auto& drawable = gameObject->addComponent<DrawableComponent>();
        
        // Set up transform
        transform.setPosition(Vector3(10.0f, 5.0f, -20.0f));
        transform.setScale(Vector3(2.0f, 2.0f, 2.0f));
        
        // Set up drawable properties
        drawable.setVisible(true);
        drawable.setCastShadows(true);
        drawable.setReceiveShadows(true);
        
        // Verify components work together
        REQUIRE(gameObject->hasComponent<TransformComponent>());
        REQUIRE(gameObject->hasComponent<DrawableComponent>());
        
        // Test component interaction
        Vector3 worldPos = drawable.getWorldPosition();
        REQUIRE(worldPos == Vector3(10.0f, 5.0f, -20.0f));
        
        // Test GameObject lifecycle
        gameObject->setActive(false);
        REQUIRE_FALSE(gameObject->isActive());
        
        gameObject->setActive(true);
        REQUIRE(gameObject->isActive());
    }
    
    SECTION("MULTIPLE_GAMEOBJECTS_WITH_COMPONENTS")
    {
        ComponentManager manager;
        
        // Create multiple GameObjects
        std::vector<std::unique_ptr<GameObject>> gameObjects;
        
        for (int i = 0; i < 5; ++i)
        {
            auto gameObject = std::make_unique<GameObject>("Object" + std::to_string(i), i, &manager);
            
            // Add components to each
            auto& transform = gameObject->addComponent<TransformComponent>();
            auto& drawable = gameObject->addComponent<DrawableComponent>();
            
            // Set unique positions
            transform.setPosition(Vector3(i * 5.0f, 0.0f, 0.0f));
            
            // Alternate visibility
            drawable.setVisible(i % 2 == 0);
            
            gameObjects.push_back(std::move(gameObject));
        }
        
        // Verify all objects created correctly
        REQUIRE(gameObjects.size() == 5);
        
        // Test each object
        for (size_t i = 0; i < gameObjects.size(); ++i)
        {
            auto& obj = gameObjects[i];
            
            REQUIRE(obj->hasComponent<TransformComponent>());
            REQUIRE(obj->hasComponent<DrawableComponent>());
            
            auto& transform = obj->getComponent<TransformComponent>();
            auto& drawable = obj->getComponent<DrawableComponent>();
            
            REQUIRE(transform.getPosition().X == i * 5.0f);
            REQUIRE(drawable.isVisible() == (i % 2 == 0));
        }
    }
    
    SECTION("COMPONENT_DEPENDENCY_RESOLUTION")
    {
        ComponentManager manager;
        GameObject gameObject("DependencyTest", 1, &manager);
        
        // Add components in different order to test dependency resolution
        auto& drawable = gameObject.addComponent<DrawableComponent>();
        auto& transform = gameObject.addComponent<TransformComponent>();
        
        // Manually wire dependency for demonstration
        // (In a full implementation, this would be automatic)
        auto transformPtr = gameObject.tryGetComponent<TransformComponent>();
        REQUIRE(transformPtr != nullptr);
        
        drawable.setTransformComponent(
            std::shared_ptr<TransformComponent>(transformPtr, [](TransformComponent*){})
        );
        
        // Test that dependency works
        transform.setPosition(Vector3(100.0f, 200.0f, 300.0f));
        
        Vector3 worldPos = drawable.getWorldPosition();
        REQUIRE(worldPos.X == 100.0f);
        REQUIRE(worldPos.Y == 200.0f);
        REQUIRE(worldPos.Z == 300.0f);
        
        // Test world matrix access
        const Matrix4& worldMatrix = drawable.getWorldMatrix();
        REQUIRE(worldMatrix != Matrix4::Identity);
    }
    
    SECTION("PERFORMANCE_WITH_MANY_COMPONENTS")
    {
        ComponentManager manager;
        std::vector<std::unique_ptr<GameObject>> gameObjects;
        
        const int objectCount = 100;
        
        // Create many objects with components
        for (int i = 0; i < objectCount; ++i)
        {
            auto gameObject = std::make_unique<GameObject>("PerfObject" + std::to_string(i), i, &manager);
            
            gameObject->addComponent<TransformComponent>();
            gameObject->addComponent<DrawableComponent>();
            
            gameObjects.push_back(std::move(gameObject));
        }
        
        // Time component access across all objects
        auto start = std::chrono::high_resolution_clock::now();
        
        for (auto& obj : gameObjects)
        {
            auto& transform = obj->getComponent<TransformComponent>();
            auto& drawable = obj->getComponent<DrawableComponent>();
            
            transform.setPosition(Vector3(1.0f, 2.0f, 3.0f));
            drawable.setVisible(true);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        // Should be very fast even with 100 objects
        REQUIRE(duration.count() < 5000); // Less than 5ms
        
        // Verify all objects still work correctly
        for (auto& obj : gameObjects)
        {
            REQUIRE(obj->hasComponent<TransformComponent>());
            REQUIRE(obj->hasComponent<DrawableComponent>());
        }
    }
}

TEST_CASE("TESTING_BENEFITS_DEMONSTRATION")
{
    SECTION("ISOLATED_COMPONENT_TESTING")
    {
        // This demonstrates how easy it now is to test components in isolation
        
        // Test TransformComponent alone
        TransformComponent transform;
        transform.setPosition(Vector3(1.0f, 2.0f, 3.0f));
        REQUIRE(transform.getPosition() == Vector3(1.0f, 2.0f, 3.0f));
        REQUIRE(transform.hasChanged());
        
        // Test DrawableComponent alone
        DrawableComponent drawable;
        drawable.setVisible(false);
        REQUIRE_FALSE(drawable.isVisible());
        
        drawable.setCastShadows(false);
        REQUIRE_FALSE(drawable.getCastShadows());
        
        // No complex setup needed - components can be tested directly!
    }
    
    SECTION("COMPONENT_INTERACTION_TESTING")
    {
        ComponentManager manager;
        GameObject gameObject("InteractionTest", 1, &manager);
        
        // Test adding multiple components
        auto& transform = gameObject.addComponent<TransformComponent>();
        auto& drawable = gameObject.addComponent<DrawableComponent>();
        
        // Test that components can interact
        transform.setPosition(Vector3(5.0f, 10.0f, 15.0f));
        
        // Wire up dependency for testing
        auto transformPtr = gameObject.tryGetComponent<TransformComponent>();
        drawable.setTransformComponent(
            std::shared_ptr<TransformComponent>(transformPtr, [](TransformComponent*){})
        );
        
        // Verify interaction works
        Vector3 worldPos = drawable.getWorldPosition();
        REQUIRE(worldPos == Vector3(5.0f, 10.0f, 15.0f));
    }
    
    SECTION("ERROR_HANDLING_TESTING")
    {
        ComponentManager manager;
        GameObject gameObject("ErrorTest", 1, &manager);
        
        // Test adding same component type twice (should throw)
        gameObject.addComponent<TransformComponent>();
        REQUIRE_THROWS_AS(gameObject.addComponent<TransformComponent>(), std::runtime_error);
        
        // Test getting non-existent component (should throw)
        REQUIRE_THROWS_AS(gameObject.getComponent<DrawableComponent>(), std::runtime_error);
        
        // Test safe component access
        auto* drawable = gameObject.tryGetComponent<DrawableComponent>();
        REQUIRE(drawable == nullptr);
        
        // Add component and verify safe access works
        gameObject.addComponent<DrawableComponent>();
        drawable = gameObject.tryGetComponent<DrawableComponent>();
        REQUIRE(drawable != nullptr);
    }
}
