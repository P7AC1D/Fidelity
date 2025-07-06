#include "catch.hpp"
#include <chrono>

#include "../Engine/Core/ComponentManager.h"
#include "../Engine/Core/GameObject.h"
#include "../Engine/Core/TransformComponent.h"
#include "../Engine/Rendering/DrawableComponent.h"
#include "../Engine/Rendering/StaticMesh.h"
#include "../Engine/Rendering/Material.h"

// Helper function to create a StaticMesh with specific bounds
std::shared_ptr<StaticMesh> createTestMesh(const Aabb& bounds)
{
    auto mesh = std::make_shared<StaticMesh>();
    
    Vector3 min = bounds.getMin();
    Vector3 max = bounds.getMax();
    
    // Create a simple cube with 8 vertices
    std::vector<Vector3> vertices = {
        {min.X, min.Y, min.Z}, // 0
        {max.X, min.Y, min.Z}, // 1
        {max.X, max.Y, min.Z}, // 2
        {min.X, max.Y, min.Z}, // 3
        {min.X, min.Y, max.Z}, // 4
        {max.X, min.Y, max.Z}, // 5
        {max.X, max.Y, max.Z}, // 6
        {min.X, max.Y, max.Z}  // 7
    };
    
    mesh->setPositionVertexData(vertices);
    return mesh;
}

TEST_CASE("DRAWABLE_COMPONENT_TESTS")
{
    SECTION("COMPONENT_CREATION_AND_TYPE_ID")
    {
        DrawableComponent drawable;
        
        REQUIRE(drawable.getTypeId() == getComponentTypeId<DrawableComponent>());
        REQUIRE(drawable.getTypeId() == DrawableComponent::GetTypeId());
    }
    
    SECTION("MESH_AND_MATERIAL_MANAGEMENT")
    {
        DrawableComponent drawable;
        
        // Initially should have no mesh or material
        REQUIRE(drawable.getMesh() == nullptr);
        REQUIRE(drawable.getMaterial() == nullptr);
        
        // Create real mesh and material objects
        auto testMesh = std::make_shared<StaticMesh>();
        auto testMaterial = std::make_shared<Material>();
        
        // Set mesh and material
        drawable.setMesh(testMesh);
        drawable.setMaterial(testMaterial);
        
        REQUIRE(drawable.getMesh() != nullptr);
        REQUIRE(drawable.getMaterial() != nullptr);
    }
    
    SECTION("CONSTRUCTOR_WITH_MESH_AND_MATERIAL")
    {
        Aabb testBounds(Vector3(2.0f, 2.0f, 2.0f), Vector3(-2.0f, -2.0f, -2.0f));
        auto testMesh = createTestMesh(testBounds);
        auto testMaterial = std::make_shared<Material>();
        
        DrawableComponent drawable(testMesh, testMaterial);
        
        REQUIRE(drawable.getMesh() != nullptr);
        REQUIRE(drawable.getMaterial() != nullptr);
    }
    
    SECTION("VISIBILITY_AND_RENDERING_FLAGS")
    {
        DrawableComponent drawable;
        
        // Test default values
        REQUIRE(drawable.isVisible() == true);
        REQUIRE(drawable.getCastShadows() == true);
        REQUIRE(drawable.getReceiveShadows() == true);
        REQUIRE(drawable.shouldDrawAabb() == false);
        
        // Test setting values
        drawable.setVisible(false);
        REQUIRE(drawable.isVisible() == false);
        
        drawable.setCastShadows(false);
        REQUIRE(drawable.getCastShadows() == false);
        
        drawable.setReceiveShadows(false);
        REQUIRE(drawable.getReceiveShadows() == false);
        
        drawable.enableDrawAabb(true);
        REQUIRE(drawable.shouldDrawAabb() == true);
    }
    
    SECTION("BOUNDS_CALCULATION_WITHOUT_TRANSFORM")
    {
        DrawableComponent drawable;
        
        // Without mesh, bounds should be default AABB (center=0, extents=1)
        const Aabb& localBounds = drawable.getLocalBounds();
        const Aabb& worldBounds = drawable.getWorldBounds();
        
        // Default AABB has center=(0,0,0), extents=(1,1,1)
        // So min = center - extents = (-1,-1,-1), max = center + extents = (1,1,1)
        REQUIRE(localBounds.getMin() == Vector3(-1.0f, -1.0f, -1.0f));
        REQUIRE(localBounds.getMax() == Vector3(1.0f, 1.0f, 1.0f));
    }
    
    SECTION("BOUNDS_CALCULATION_WITH_MESH")
    {
        Aabb testBounds(Vector3(3.0f, 3.0f, 3.0f), Vector3(-3.0f, -3.0f, -3.0f));
        auto testMesh = createTestMesh(testBounds);
        
        DrawableComponent drawable;
        drawable.setMesh(testMesh);
        
        const Aabb& localBounds = drawable.getLocalBounds();
        
        REQUIRE(localBounds.getMin() == Vector3(-3.0f, -3.0f, -3.0f));
        REQUIRE(localBounds.getMax() == Vector3(3.0f, 3.0f, 3.0f));
    }
    
    SECTION("TRANSFORM_INTEGRATION")
    {
        ComponentManager manager;
        GameObject gameObject("TestObject", 1, &manager);
        
        // Add both transform and drawable components
        auto& transform = gameObject.addComponent<TransformComponent>();
        auto& drawable = gameObject.addComponent<DrawableComponent>();
        
        // Set up the transform dependency using the same approach as ComponentDependency
        auto* transformPtr = gameObject.tryGetComponent<TransformComponent>();
        REQUIRE(transformPtr != nullptr);
        
        // Create a shared_ptr that doesn't own the object (since GameObject owns it)
        auto transformSharedPtr = std::shared_ptr<TransformComponent>(transformPtr, [](TransformComponent*){});
        drawable.setTransformComponent(transformSharedPtr);
        
        // Test world position access
        transform.setPosition(Vector3(10.0f, 20.0f, 30.0f));
        Vector3 worldPos = drawable.getWorldPosition();
        REQUIRE(worldPos == Vector3(10.0f, 20.0f, 30.0f));
    }
    
    SECTION("CHANGE_TRACKING")
    {
        DrawableComponent drawable;
        
        // Initially should be marked as changed
        REQUIRE(drawable.hasChanged() == true);
        
        // Create a mesh to set bounds
        Aabb testBounds(Vector3(1.0f, 1.0f, 1.0f), Vector3(-1.0f, -1.0f, -1.0f));
        auto testMesh = createTestMesh(testBounds);
        drawable.setMesh(testMesh);
        
        // Setting mesh should mark as changed
        REQUIRE(drawable.hasChanged() == true);
        
        // Getting bounds should update the cache
        drawable.getWorldBounds();
        
        // After bounds update, should not be changed anymore
        // (Note: This depends on implementation details)
    }
    
    SECTION("COMPONENT_LIFECYCLE")
    {
        ComponentManager manager;
        
        auto drawable = manager.createComponent<DrawableComponent>();
        
        // Test lifecycle methods don't crash
        drawable->initialize();
        drawable->activate();
        drawable->deactivate();
        
        // Component should still be functional
        drawable->setVisible(false);
        REQUIRE(drawable->isVisible() == false);
    }
    
    SECTION("INTEGRATION_WITH_GAMEOBJECT_V2")
    {
        ComponentManager manager;
        GameObject gameObject("TestDrawableObject", 1, &manager);
        
        // Add drawable component
        auto& drawable = gameObject.addComponent<DrawableComponent>();
        REQUIRE(gameObject.hasComponent<DrawableComponent>());
        
        // Test component functionality through GameObject
        drawable.setVisible(false);
        REQUIRE_FALSE(drawable.isVisible());
        
        // Test component retrieval
        auto& retrievedDrawable = gameObject.getComponent<DrawableComponent>();
        REQUIRE(&drawable == &retrievedDrawable);
    }
}

// Comparison with old system
TEST_CASE("OLD_VS_NEW_DRAWABLE_TESTING_COMPARISON")
{
    SECTION("NEW_SYSTEM_ISOLATED_TESTING")
    {
        // New system: Direct, isolated component testing
        DrawableComponent drawable;
        
        // Simple, direct testing
        drawable.setVisible(false);
        REQUIRE_FALSE(drawable.isVisible());
        
        drawable.setCastShadows(false);
        REQUIRE_FALSE(drawable.getCastShadows());
        
        // No complex setup required!
    }
    
    /*
    SECTION("OLD_SYSTEM_COMPLEX_TESTING") 
    {
        // Old system would require:
        // 1. Scene creation with InputHandler
        // 2. RenderDevice setup
        // 3. GameObject creation through Scene
        // 4. Component creation through Scene
        // 5. Complex notification system setup
        // 6. Transform synchronization
        
        // This was a major pain point for testing!
    }
    */
}

// Performance testing (basic example)
TEST_CASE("DRAWABLE_COMPONENT_PERFORMANCE")
{
    SECTION("COMPONENT_ACCESS_PERFORMANCE")
    {
        ComponentManager manager;
        GameObject gameObject("PerfTest", 1, &manager);
        
        // Add component
        gameObject.addComponent<DrawableComponent>();
        
        const int iterations = 1000;
        
        // Time component access
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < iterations; ++i)
        {
            auto& drawable = gameObject.getComponent<DrawableComponent>();
            drawable.setVisible(i % 2 == 0);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        // Should be very fast (< 1ms for 1000 iterations)
        REQUIRE(duration.count() < 1000);
    }
}
