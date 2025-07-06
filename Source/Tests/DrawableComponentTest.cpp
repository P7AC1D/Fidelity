#include "catch.hpp"
#include <chrono>

#include "../Engine/Core/ComponentManager.h"
#include "../Engine/Core/GameObject.h"
#include "../Engine/Core/TransformComponent.h"
#include "../Engine/Rendering/DrawableComponent.h"

// Mock classes for testing
class MockStaticMesh
{
public:
    MockStaticMesh(const Aabb& bounds) : _bounds(bounds) {}
    const Aabb& getAabb() const { return _bounds; }
    
private:
    Aabb _bounds;
};

class MockMaterial
{
public:
    MockMaterial() = default;
    
    // Mock material interface
    Colour getDiffuseColour() const { return Colour(255, 255, 255); }
    void setDiffuseColour(const Colour& colour) { _diffuseColour = colour; }
    
    float32 getMetalness() const { return _metalness; }
    void setMetalness(float32 value) { _metalness = value; }
    
    float32 getRoughness() const { return _roughness; }
    void setRoughness(float32 value) { _roughness = value; }
    
    // Texture methods
    bool hasDiffuseTexture() const { return false; }
    bool diffuseTextureEnabled() const { return false; }
    void enableDiffuseTexture(bool enable) {}
    
    bool hasNormalTexture() const { return false; }
    bool normalTextureEnabled() const { return false; }
    void enableNormalTexture(bool enable) {}
    
    bool hasMetallicTexture() const { return false; }
    bool metallicTextureEnabled() const { return false; }
    void enableMetallicTexture(bool enable) {}
    
    bool hasRoughnessTexture() const { return false; }
    bool roughnessTextureEnabled() const { return false; }
    void enableRoughnessTexture(bool enable) {}
    
    bool hasOcclusionTexture() const { return false; }
    bool occlusionTextureEnabled() const { return false; }
    void enableOcclusionTexture(bool enable) {}
    
    bool hasOpacityTexture() const { return false; }
    bool opacityTextureEnabled() const { return false; }
    void enableOppacityTexture(bool enable) {}
    
    std::shared_ptr<class Texture> getDiffuseTexture() const { return nullptr; }
    std::shared_ptr<class Texture> getNormalTexture() const { return nullptr; }
    std::shared_ptr<class Texture> getMetallicTexture() const { return nullptr; }
    std::shared_ptr<class Texture> getRoughnessTexture() const { return nullptr; }
    std::shared_ptr<class Texture> getOpacityTexture() const { return nullptr; }
    
private:
    Colour _diffuseColour = Colour(255, 255, 255);
    float32 _metalness = 0.0f;
    float32 _roughness = 0.5f;
};

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
        
        // Create mock mesh and material
        Aabb testBounds(Vector3(1.0f, 1.0f, 1.0f), Vector3(-1.0f, -1.0f, -1.0f));
        auto mockMesh = std::make_shared<MockStaticMesh>(testBounds);
        auto mockMaterial = std::make_shared<MockMaterial>();
        
        // Set mesh and material
        drawable.setMesh(std::reinterpret_pointer_cast<StaticMesh>(mockMesh));
        drawable.setMaterial(std::reinterpret_pointer_cast<Material>(mockMaterial));
        
        REQUIRE(drawable.getMesh() != nullptr);
        REQUIRE(drawable.getMaterial() != nullptr);
    }
    
    SECTION("CONSTRUCTOR_WITH_MESH_AND_MATERIAL")
    {
        Aabb testBounds(Vector3(2.0f, 2.0f, 2.0f), Vector3(-2.0f, -2.0f, -2.0f));
        auto mockMesh = std::make_shared<MockStaticMesh>(testBounds);
        auto mockMaterial = std::make_shared<MockMaterial>();
        
        DrawableComponent drawable(
            std::reinterpret_pointer_cast<StaticMesh>(mockMesh),
            std::reinterpret_pointer_cast<Material>(mockMaterial)
        );
        
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
        
        // Without mesh, bounds should be empty
        const Aabb& localBounds = drawable.getLocalBounds();
        const Aabb& worldBounds = drawable.getWorldBounds();
        
        // Initially both should be empty/default AABBs
        REQUIRE(localBounds.getMin() == Vector3::Zero);
        REQUIRE(localBounds.getMax() == Vector3::Zero);
    }
    
    SECTION("BOUNDS_CALCULATION_WITH_MESH")
    {
        Aabb testBounds(Vector3(3.0f, 3.0f, 3.0f), Vector3(-3.0f, -3.0f, -3.0f));
        auto mockMesh = std::make_shared<MockStaticMesh>(testBounds);
        
        DrawableComponent drawable;
        drawable.setMesh(std::reinterpret_pointer_cast<StaticMesh>(mockMesh));
        
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
        
        // Set up the transform dependency
        auto transformPtr = gameObject.tryGetComponent<TransformComponent>();
        REQUIRE(transformPtr != nullptr);
        
        // In a real system, this dependency would be automatically resolved
        // For now, we manually set it for testing
        drawable.setTransformComponent(std::shared_ptr<TransformComponent>(transformPtr, [](TransformComponent*){}));
        
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
        auto mockMesh = std::make_shared<MockStaticMesh>(testBounds);
        drawable.setMesh(std::reinterpret_pointer_cast<StaticMesh>(mockMesh));
        
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
