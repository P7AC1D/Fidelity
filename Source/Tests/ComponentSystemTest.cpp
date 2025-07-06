#include "catch.hpp"

#include "../Engine/Core/ComponentManager.h"
#include "../Engine/Core/GameObject.h"
#include "../Engine/Core/TransformComponent.h"

TEST_CASE("MODERN_COMPONENT_SYSTEM_TESTS")
{
    SECTION("COMPONENT_TYPE_ID_GENERATION")
    {
        // Test that component type IDs are unique and consistent
        ComponentTypeId id1 = getComponentTypeId<TransformComponent>();
        ComponentTypeId id2 = getComponentTypeId<TransformComponent>();
        
        REQUIRE(id1 == id2); // Same type should have same ID
        
        // Different calls should return same ID
        ComponentTypeId id3 = TransformComponent::GetTypeId();
        REQUIRE(id1 == id3);
    }
    
    SECTION("COMPONENT_MANAGER_CREATION")
    {
        ComponentManager manager;
        
        // Test component creation
        auto transform = manager.createComponent<TransformComponent>();
        REQUIRE(transform != nullptr);
        REQUIRE(transform->getTypeId() == getComponentTypeId<TransformComponent>());
    }
    
    SECTION("GAMEOBJECT_V2_COMPONENT_MANAGEMENT")
    {
        ComponentManager manager;
        GameObject gameObject("TestObject", 1, &manager);
        
        // Test adding components
        auto& transform = gameObject.addComponent<TransformComponent>();
        REQUIRE(gameObject.hasComponent<TransformComponent>());
        
        // Test getting components
        auto& retrievedTransform = gameObject.getComponent<TransformComponent>();
        REQUIRE(&transform == &retrievedTransform);
        
        // Test component functionality
        transform.setPosition(Vector3(1.0f, 2.0f, 3.0f));
        REQUIRE(transform.getPosition() == Vector3(1.0f, 2.0f, 3.0f));
        REQUIRE(transform.hasChanged());
    }
    
    SECTION("TRANSFORM_COMPONENT_FUNCTIONALITY")
    {
        TransformComponent transform;
        
        // Test initial state
        REQUIRE(transform.getPosition() == Vector3::Zero);
        REQUIRE(transform.getRotation() == Quaternion::Identity);
        REQUIRE(transform.getScale() == Vector3::Identity);
        
        // Test position
        transform.setPosition(Vector3(10.0f, 20.0f, 30.0f));
        REQUIRE(transform.getPosition() == Vector3(10.0f, 20.0f, 30.0f));
        REQUIRE(transform.hasChanged());
        
        // Test translation
        transform.translate(Vector3(5.0f, 0.0f, 0.0f));
        REQUIRE(transform.getPosition() == Vector3(15.0f, 20.0f, 30.0f));
        
        // Test scale
        transform.setScale(Vector3(2.0f, 2.0f, 2.0f));
        REQUIRE(transform.getScale() == Vector3(2.0f, 2.0f, 2.0f));
        
        // Test world matrix calculation
        const Matrix4& worldMatrix = transform.getWorldMatrix();
        REQUIRE(worldMatrix != Matrix4::Identity); // Should be different due to transforms
    }
    
    SECTION("COMPONENT_LIFECYCLE")
    {
        ComponentManager manager;
        
        auto transform = manager.createComponent<TransformComponent>();
        
        // Test lifecycle methods don't crash
        transform->initialize();
        transform->activate();
        transform->deactivate();
        
        // Component should still be functional
        transform->setPosition(Vector3(1.0f, 1.0f, 1.0f));
        REQUIRE(transform->getPosition() == Vector3(1.0f, 1.0f, 1.0f));
    }
    
    SECTION("MULTIPLE_COMPONENTS_SAME_TYPE_ERROR")
    {
        ComponentManager manager;
        GameObject gameObject("TestObject", 1, &manager);
        
        // First component should succeed
        gameObject.addComponent<TransformComponent>();
        REQUIRE(gameObject.hasComponent<TransformComponent>());
        
        // Second component of same type should throw
        REQUIRE_THROWS_AS(gameObject.addComponent<TransformComponent>(), std::runtime_error);
    }
    
    SECTION("COMPONENT_REMOVAL")
    {
        ComponentManager manager;
        GameObject gameObject("TestObject", 1, &manager);
        
        // Add component
        gameObject.addComponent<TransformComponent>();
        REQUIRE(gameObject.hasComponent<TransformComponent>());
        
        // Remove component
        bool removed = gameObject.removeComponent<TransformComponent>();
        REQUIRE(removed);
        REQUIRE_FALSE(gameObject.hasComponent<TransformComponent>());
        
        // Try to remove again - should return false
        bool removedAgain = gameObject.removeComponent<TransformComponent>();
        REQUIRE_FALSE(removedAgain);
    }
    
    SECTION("TRY_GET_COMPONENT")
    {
        ComponentManager manager;
        GameObject gameObject("TestObject", 1, &manager);
        
        // Try get when component doesn't exist
        auto* transform = gameObject.tryGetComponent<TransformComponent>();
        REQUIRE(transform == nullptr);
        
        // Add component and try get again
        gameObject.addComponent<TransformComponent>();
        transform = gameObject.tryGetComponent<TransformComponent>();
        REQUIRE(transform != nullptr);
    }
    
    SECTION("GAMEOBJECT_ACTIVATION")
    {
        ComponentManager manager;
        GameObject gameObject("TestObject", 1, &manager);
        
        // GameObject should be active by default
        REQUIRE(gameObject.isActive());
        
        // Add component while active
        auto& transform = gameObject.addComponent<TransformComponent>();
        
        // Deactivate GameObject
        gameObject.setActive(false);
        REQUIRE_FALSE(gameObject.isActive());
        
        // Reactivate
        gameObject.setActive(true);
        REQUIRE(gameObject.isActive());
    }
}

// Performance comparison test (commented out for now)
/*
TEST_CASE("PERFORMANCE_COMPARISON")
{
    SECTION("OLD_VS_NEW_COMPONENT_ACCESS")
    {
        // This would test performance differences between:
        // - Old: dynamic_cast lookup vs New: direct type-safe lookup
        // - Old: raw pointer management vs New: smart pointer management
        // - Old: forced inheritance vs New: optional interfaces
        
        const int iterations = 10000;
        
        // Time old system component access
        auto start = std::chrono::high_resolution_clock::now();
        // ... old system test code ...
        auto oldTime = std::chrono::high_resolution_clock::now() - start;
        
        // Time new system component access
        start = std::chrono::high_resolution_clock::now();
        // ... new system test code ...
        auto newTime = std::chrono::high_resolution_clock::now() - start;
        
        // New system should be faster or at least not significantly slower
        REQUIRE(newTime <= oldTime * 1.2f); // Allow 20% slower at most
    }
}
*/
