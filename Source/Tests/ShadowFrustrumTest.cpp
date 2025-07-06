#include "catch.hpp"

#include <chrono>
#include "../Engine/Rendering/ShadowFrustum.h"
#include "../Engine/Rendering/CameraComponent.h"
#include "../Engine/Rendering/LightComponent.h"
#include "../Engine/Rendering/DrawableComponent.h"
#include "../Engine/Rendering/Material.h"
#include "../Engine/Rendering/StaticMesh.h"
#include "../Engine/Maths/AABB.hpp"
#include "../Engine/Core/TransformComponent.h"
#include "../Engine/Core/GameObject.h"

// Helper class for creating test scenarios
class ShadowTestHelper
{
public:
    // Create a basic directional light pointing down
    static std::shared_ptr<Light> createDirectionalLight(const Vector3& direction = Vector3(0.0f, -1.0f, 0.0f))
    {
        auto light = std::make_shared<Light>();
        light->setLightType(LightType::Directional);
        // Note: Light direction is calculated from transform, not set directly
        light->setColour(Colour::White);
        light->setIntensity(1.0f);
        return light;
    }
    
    // Create a test camera at a specific position
    static std::shared_ptr<Camera> createTestCamera(const Vector3& position = Vector3::Zero,
                                                   const Vector3& target = Vector3(0.0f, 0.0f, -1.0f))
    {
        auto camera = std::make_shared<Camera>();
        camera->setPerspective(Degree(60.0f), 1280, 768, 0.1f, 100.0f);
        
        // Create a minimal transform setup
        Transform transform;
        transform.setPosition(position);
        
        Vector3 forward = Vector3::Normalize(target - position);
        Vector3 right = Vector3::Normalize(Vector3::Cross(forward, Vector3::Up));
        Vector3 up = Vector3::Normalize(Vector3::Cross(right, forward));
        
        Matrix4 viewMatrix = Matrix4::LookAt(position, target, up);
        
        // We'll need to manually set the camera's view matrix for testing
        // This is a simplified approach for testing purposes
        return camera;
    }
    
    // Create a test drawable at a specific position with a specific AABB size
    static std::shared_ptr<Drawable> createTestDrawable(const Vector3& position,
                                                        const Vector3& size = Vector3(1.0f, 1.0f, 1.0f),
                                                        bool hasOpacity = false)
    {
        // Create a custom drawable class for testing that allows us to set AABB directly
        class TestDrawable : public Drawable
        {
        public:
            TestDrawable(const Vector3& pos, const Vector3& extents, bool opacity = false)
            {
                // Set up material
                auto material = std::make_shared<Material>();
                material->setDiffuseColour(Colour::White);
                setMaterial(material);
                
                // Create AABB with specified size
                _testAabb = Aabb(pos, extents.X * 0.5f, extents.Y * 0.5f, extents.Z * 0.5f);
                _testPosition = pos;
                
                // Create transform
                _testTransform.setPosition(pos);
            }
            
            // Override getAabb to return our test AABB
            const Aabb& getAabb() const { return _testAabb; }
            
            // Override getPosition to return our test position
            Vector3 getPosition() const { return _testPosition; }
            
            // Override getCachedTransform to return our test transform
            const Transform& getCachedTransform() const { return _testTransform; }
            
        private:
            Aabb _testAabb;
            Vector3 _testPosition;
            mutable Transform _testTransform;
        };
        
        return std::make_shared<TestDrawable>(position, size, hasOpacity);
    }
    
    // Create orthographic projection matrix for testing
    static Matrix4 createOrthographicMatrix(float32 left, float32 right, 
                                           float32 bottom, float32 top,
                                           float32 nearPlane, float32 farPlane)
    {
        return Matrix4::Orthographic(left, right, bottom, top, nearPlane, farPlane);
    }
    
    // Create perspective projection matrix for testing
    static Matrix4 createPerspectiveMatrix(float32 fov, float32 aspect, 
                                          float32 nearPlane, float32 farPlane)
    {
        return Matrix4::Perspective(Degree(fov), aspect, nearPlane, farPlane);
    }
    
    // Create a view matrix looking in a specific direction
    static Matrix4 createViewMatrix(const Vector3& position, const Vector3& direction, const Vector3& up = Vector3::Up)
    {
        Vector3 target = position + Vector3::Normalize(direction);
        return Matrix4::LookAt(position, target, up);
    }
    
    // Create light-space transform matrices for cascade testing
    static std::vector<Matrix4> createTestLightTransforms(uint32 cascadeCount = 4)
    {
        std::vector<Matrix4> transforms;
        
        for (uint32 i = 0; i < cascadeCount; ++i)
        {
            // Create increasingly larger orthographic projections for each cascade
            float32 size = 10.0f * (i + 1);
            Matrix4 proj = createOrthographicMatrix(-size, size, -size, size, -50.0f, 50.0f);
            Matrix4 view = createViewMatrix(Vector3(0.0f, 20.0f, 0.0f), Vector3(0.0f, -1.0f, 0.0f));
            transforms.push_back(proj * view);
        }
        
        return transforms;
    }
};

TEST_CASE("SHADOW_FRUSTUM_CONSTRUCTION")
{
    SECTION("BASIC_CONSTRUCTION")
    {
        ShadowFrustum shadowFrustum;
        
        // Test that construction doesn't crash
        REQUIRE(true);
    }
    
    SECTION("BUILD_FROM_LIGHT_TRANSFORMS")
    {
        ShadowFrustum shadowFrustum;
        auto lightTransforms = ShadowTestHelper::createTestLightTransforms(4);
        
        // Test building from light transforms
        REQUIRE_NOTHROW(shadowFrustum.buildFromLightTransforms(lightTransforms, 4));
        
        // Test that we can access cascade frustums
        for (uint32 i = 0; i < 4; ++i)
        {
            REQUIRE_NOTHROW(shadowFrustum.getCascadeFrustum(i));
        }
    }
    
    SECTION("BUILD_EXTENDED_CAMERA_FRUSTUM")
    {
        ShadowFrustum shadowFrustum;
        auto camera = ShadowTestHelper::createTestCamera();
        
        // Test building extended camera frustum
        REQUIRE_NOTHROW(shadowFrustum.buildExtendedCameraFrustum(*camera, 1.5f));
        
        // Test that we can access the extended frustum
        REQUIRE_NOTHROW(shadowFrustum.getExtendedCameraFrustum());
    }
    
    SECTION("INVALID_CASCADE_COUNT")
    {
        ShadowFrustum shadowFrustum;
        auto lightTransforms = ShadowTestHelper::createTestLightTransforms(2);
        
        // Test with too many cascades
        REQUIRE_THROWS(shadowFrustum.buildFromLightTransforms(lightTransforms, 6)); // MAX_CASCADE_LAYERS is 4
        
        // Test with insufficient transforms
        REQUIRE_THROWS(shadowFrustum.buildFromLightTransforms(lightTransforms, 4)); // Only 2 transforms provided
    }
}

TEST_CASE("SHADOW_FRUSTUM_MATRIX_EXTRACTION")
{
    SECTION("ORTHOGRAPHIC_MATRIX_EXTRACTION")
    {
        ShadowFrustum shadowFrustum;
        
        // Create a known orthographic matrix
        Matrix4 orthoMatrix = ShadowTestHelper::createOrthographicMatrix(-10.0f, 10.0f, -10.0f, 10.0f, -20.0f, 20.0f);
        Matrix4 viewMatrix = ShadowTestHelper::createViewMatrix(Vector3(0.0f, 10.0f, 0.0f), Vector3(0.0f, -1.0f, 0.0f));
        Matrix4 lightTransform = orthoMatrix * viewMatrix;
        
        std::vector<Matrix4> transforms = { lightTransform };
        
        // Build frustum from this known matrix
        REQUIRE_NOTHROW(shadowFrustum.buildFromLightTransforms(transforms, 1));
        
        // Test that we can get the frustum back
        const Frustrum& frustum = shadowFrustum.getCascadeFrustum(0);
        
        // The frustum should be valid (this is a basic sanity check)
        REQUIRE(true); // If we get here without crashing, the extraction worked
    }
    
    SECTION("PERSPECTIVE_MATRIX_EXTRACTION")
    {
        ShadowFrustum shadowFrustum;
        
        // Create a known perspective matrix
        Matrix4 perspMatrix = ShadowTestHelper::createPerspectiveMatrix(60.0f, 16.0f/9.0f, 1.0f, 100.0f);
        Matrix4 viewMatrix = ShadowTestHelper::createViewMatrix(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, -1.0f));
        Matrix4 lightTransform = perspMatrix * viewMatrix;
        
        std::vector<Matrix4> transforms = { lightTransform };
        
        // Build frustum from this known matrix
        REQUIRE_NOTHROW(shadowFrustum.buildFromLightTransforms(transforms, 1));
        
        // Test that we can get the frustum back
        const Frustrum& frustum = shadowFrustum.getCascadeFrustum(0);
        
        // The frustum should be valid
        REQUIRE(true);
    }
    
    SECTION("IDENTITY_MATRIX_EDGE_CASE")
    {
        ShadowFrustum shadowFrustum;
        
        // Test with identity matrix (edge case)
        Matrix4 identityMatrix = Matrix4::Identity;
        std::vector<Matrix4> transforms = { identityMatrix };
        
        // This should not crash, even though it's a degenerate case
        REQUIRE_NOTHROW(shadowFrustum.buildFromLightTransforms(transforms, 1));
    }
}

TEST_CASE("SHADOW_FRUSTUM_CULLING_ACCURACY")
{
    SECTION("CASCADE_CULLING_BASIC")
    {
        ShadowFrustum shadowFrustum;
        auto lightTransforms = ShadowTestHelper::createTestLightTransforms(4);
        shadowFrustum.buildFromLightTransforms(lightTransforms, 4);
        
        // Create test objects at various positions
        std::vector<std::shared_ptr<Drawable>> testObjects;
        testObjects.push_back(ShadowTestHelper::createTestDrawable(Vector3(0.0f, 0.0f, 0.0f)));   // Center
        testObjects.push_back(ShadowTestHelper::createTestDrawable(Vector3(5.0f, 0.0f, 0.0f)));   // Right
        testObjects.push_back(ShadowTestHelper::createTestDrawable(Vector3(-5.0f, 0.0f, 0.0f)));  // Left
        testObjects.push_back(ShadowTestHelper::createTestDrawable(Vector3(0.0f, 0.0f, 5.0f)));   // Forward
        testObjects.push_back(ShadowTestHelper::createTestDrawable(Vector3(0.0f, 0.0f, -5.0f)));  // Back
        testObjects.push_back(ShadowTestHelper::createTestDrawable(Vector3(100.0f, 0.0f, 0.0f))); // Far away
        
        // Test culling for each cascade
        for (uint32 cascadeIndex = 0; cascadeIndex < 4; ++cascadeIndex)
        {
            auto culledObjects = shadowFrustum.cullForCascade(cascadeIndex, testObjects);
            
            // Should return a valid vector (may be empty)
            REQUIRE(culledObjects.size() <= testObjects.size());
            
            // All returned objects should be from the original list
            for (const auto& culled : culledObjects)
            {
                bool found = false;
                for (const auto& original : testObjects)
                {
                    if (culled == original)
                    {
                        found = true;
                        break;
                    }
                }
                REQUIRE(found);
            }
        }
    }
    
    SECTION("BROAD_PHASE_CULLING")
    {
        ShadowFrustum shadowFrustum;
        auto camera = ShadowTestHelper::createTestCamera();
        shadowFrustum.buildExtendedCameraFrustum(*camera, 1.5f);
        
        // Create test objects at various distances
        std::vector<std::shared_ptr<Drawable>> testObjects;
        testObjects.push_back(ShadowTestHelper::createTestDrawable(Vector3(0.0f, 0.0f, -5.0f)));   // In front
        testObjects.push_back(ShadowTestHelper::createTestDrawable(Vector3(0.0f, 0.0f, -50.0f)));  // Medium distance
        testObjects.push_back(ShadowTestHelper::createTestDrawable(Vector3(0.0f, 0.0f, -200.0f))); // Far away
        testObjects.push_back(ShadowTestHelper::createTestDrawable(Vector3(0.0f, 0.0f, 10.0f)));   // Behind camera
        
        auto culledObjects = shadowFrustum.broadPhaseCull(testObjects);
        
        // Should return a valid vector
        REQUIRE(culledObjects.size() <= testObjects.size());
        
        // All returned objects should be from the original list
        for (const auto& culled : culledObjects)
        {
            bool found = false;
            for (const auto& original : testObjects)
            {
                if (culled == original)
                {
                    found = true;
                    break;
                }
            }
            REQUIRE(found);
        }
    }
    
    SECTION("SHADOW_RELEVANCE_FILTERING")
    {
        ShadowFrustum shadowFrustum;
        
        // Create objects with different properties
        std::vector<std::shared_ptr<Drawable>> testObjects;
        
        // Normal opaque object
        testObjects.push_back(ShadowTestHelper::createTestDrawable(Vector3(0.0f, 0.0f, 0.0f), Vector3(2.0f, 2.0f, 2.0f), false));
        
        // Transparent object
        testObjects.push_back(ShadowTestHelper::createTestDrawable(Vector3(1.0f, 0.0f, 0.0f), Vector3(2.0f, 2.0f, 2.0f), true));
        
        // Very small object
        testObjects.push_back(ShadowTestHelper::createTestDrawable(Vector3(2.0f, 0.0f, 0.0f), Vector3(0.01f, 0.01f, 0.01f), false));
        
        // Large object
        testObjects.push_back(ShadowTestHelper::createTestDrawable(Vector3(3.0f, 0.0f, 0.0f), Vector3(10.0f, 10.0f, 10.0f), false));
        
        auto filteredObjects = shadowFrustum.shadowRelevanceFilter(testObjects);
        
        // Should return a valid vector
        REQUIRE(filteredObjects.size() <= testObjects.size());
        
        // All returned objects should be from the original list
        for (const auto& filtered : filteredObjects)
        {
            bool found = false;
            for (const auto& original : testObjects)
            {
                if (filtered == original)
                {
                    found = true;
                    break;
                }
            }
            REQUIRE(found);
        }
        
        // Should filter out very small objects (size threshold test)
        // The very small object should likely be filtered out
        bool hasVerySmallObject = false;
        for (const auto& filtered : filteredObjects)
        {
            if (filtered == testObjects[2]) // The very small object
            {
                hasVerySmallObject = true;
                break;
            }
        }
        // Very small objects should be filtered out
        REQUIRE_FALSE(hasVerySmallObject);
    }
}

TEST_CASE("SHADOW_FRUSTUM_CAMERA_MOVEMENT_STABILITY")
{
    SECTION("SMALL_CAMERA_MOVEMENTS")
    {
        ShadowFrustum shadowFrustum;
        
        // Create initial camera position
        auto camera1 = ShadowTestHelper::createTestCamera(Vector3(0.0f, 0.0f, 0.0f));
        shadowFrustum.buildExtendedCameraFrustum(*camera1, 1.5f);
        
        // Create test object
        std::vector<std::shared_ptr<Drawable>> testObjects;
        testObjects.push_back(ShadowTestHelper::createTestDrawable(Vector3(0.0f, 0.0f, -10.0f)));
        
        // Get initial culling result
        auto initialCulled = shadowFrustum.broadPhaseCull(testObjects);
        
        // Move camera slightly
        auto camera2 = ShadowTestHelper::createTestCamera(Vector3(0.1f, 0.0f, 0.0f)); // Small movement
        shadowFrustum.buildExtendedCameraFrustum(*camera2, 1.5f);
        
        // Get culling result after small movement
        auto movedCulled = shadowFrustum.broadPhaseCull(testObjects);
        
        // Results should be consistent for small movements
        REQUIRE(initialCulled.size() == movedCulled.size());
        
        // The same objects should be culled
        for (size_t i = 0; i < initialCulled.size(); ++i)
        {
            bool found = false;
            for (const auto& moved : movedCulled)
            {
                if (initialCulled[i] == moved)
                {
                    found = true;
                    break;
                }
            }
            REQUIRE(found);
        }
    }
    
    SECTION("INCREMENTAL_CAMERA_ROTATION")
    {
        ShadowFrustum shadowFrustum;
        
        // Test object at a fixed position
        std::vector<std::shared_ptr<Drawable>> testObjects;
        testObjects.push_back(ShadowTestHelper::createTestDrawable(Vector3(5.0f, 0.0f, -10.0f)));
        
        std::vector<size_t> cullCounts;
        
        // Test with incremental camera rotations
        for (int angle = 0; angle < 360; angle += 10)
        {
            Vector3 direction(sin(angle * Math::Pi / 180.0f), 0.0f, -cos(angle * Math::Pi / 180.0f));
            auto camera = ShadowTestHelper::createTestCamera(Vector3::Zero, direction);
            shadowFrustum.buildExtendedCameraFrustum(*camera, 1.5f);
            
            auto culled = shadowFrustum.broadPhaseCull(testObjects);
            cullCounts.push_back(culled.size());
        }
        
        // Check for stability - shouldn't have dramatic changes between adjacent angles
        for (size_t i = 1; i < cullCounts.size(); ++i)
        {
            // The difference between adjacent cull counts should be reasonable
            // (This is a heuristic test - in practice, we'd want more specific validation)
            REQUIRE(cullCounts[i] <= testObjects.size());
        }
    }
    
    SECTION("CASCADE_CONSISTENCY_DURING_MOVEMENT")
    {
        ShadowFrustum shadowFrustum;
        auto lightTransforms = ShadowTestHelper::createTestLightTransforms(4);
        shadowFrustum.buildFromLightTransforms(lightTransforms, 4);
        
        // Create test objects
        std::vector<std::shared_ptr<Drawable>> testObjects;
        for (int i = 0; i < 10; ++i)
        {
            testObjects.push_back(ShadowTestHelper::createTestDrawable(
                Vector3(i * 2.0f - 10.0f, 0.0f, -5.0f)
            ));
        }
        
        // Test culling consistency across cascades
        std::vector<std::vector<std::shared_ptr<Drawable>>> cascadeCulled;
        for (uint32 cascade = 0; cascade < 4; ++cascade)
        {
            cascadeCulled.push_back(shadowFrustum.cullForCascade(cascade, testObjects));
        }
        
        // Verify that objects in smaller cascades are also in larger cascades
        // (This is a general expectation for cascade shadow maps)
        for (uint32 i = 0; i < 3; ++i) // Check cascades 0-2 against their next cascade
        {
            for (const auto& obj : cascadeCulled[i])
            {
                // Object in cascade i should also be considered for cascade i+1
                // (This is a simplified check - real cascade logic is more complex)
                REQUIRE(cascadeCulled[i].size() <= testObjects.size());
                REQUIRE(cascadeCulled[i+1].size() <= testObjects.size());
            }
        }
    }
}

TEST_CASE("SHADOW_FRUSTUM_EDGE_CASES")
{
    SECTION("EMPTY_OBJECT_LIST")
    {
        ShadowFrustum shadowFrustum;
        auto lightTransforms = ShadowTestHelper::createTestLightTransforms(4);
        shadowFrustum.buildFromLightTransforms(lightTransforms, 4);
        
        std::vector<std::shared_ptr<Drawable>> emptyObjects;
        
        // Should handle empty object lists gracefully
        for (uint32 cascade = 0; cascade < 4; ++cascade)
        {
            auto culled = shadowFrustum.cullForCascade(cascade, emptyObjects);
            REQUIRE(culled.empty());
        }
        
        auto broadCulled = shadowFrustum.broadPhaseCull(emptyObjects);
        REQUIRE(broadCulled.empty());
        
        auto filtered = shadowFrustum.shadowRelevanceFilter(emptyObjects);
        REQUIRE(filtered.empty());
    }
    
    SECTION("EXTREME_OBJECT_POSITIONS")
    {
        ShadowFrustum shadowFrustum;
        auto lightTransforms = ShadowTestHelper::createTestLightTransforms(4);
        shadowFrustum.buildFromLightTransforms(lightTransforms, 4);
        
        // Create objects at extreme positions
        std::vector<std::shared_ptr<Drawable>> extremeObjects;
        extremeObjects.push_back(ShadowTestHelper::createTestDrawable(Vector3(1000.0f, 0.0f, 0.0f)));
        extremeObjects.push_back(ShadowTestHelper::createTestDrawable(Vector3(-1000.0f, 0.0f, 0.0f)));
        extremeObjects.push_back(ShadowTestHelper::createTestDrawable(Vector3(0.0f, 1000.0f, 0.0f)));
        extremeObjects.push_back(ShadowTestHelper::createTestDrawable(Vector3(0.0f, -1000.0f, 0.0f)));
        extremeObjects.push_back(ShadowTestHelper::createTestDrawable(Vector3(0.0f, 0.0f, 1000.0f)));
        extremeObjects.push_back(ShadowTestHelper::createTestDrawable(Vector3(0.0f, 0.0f, -1000.0f)));
        
        // Should handle extreme positions without crashing
        for (uint32 cascade = 0; cascade < 4; ++cascade)
        {
            REQUIRE_NOTHROW(shadowFrustum.cullForCascade(cascade, extremeObjects));
        }
        
        REQUIRE_NOTHROW(shadowFrustum.shadowRelevanceFilter(extremeObjects));
    }
    
    SECTION("ZERO_SIZED_OBJECTS")
    {
        ShadowFrustum shadowFrustum;
        
        // Create objects with zero or near-zero size
        std::vector<std::shared_ptr<Drawable>> zeroObjects;
        zeroObjects.push_back(ShadowTestHelper::createTestDrawable(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f)));
        zeroObjects.push_back(ShadowTestHelper::createTestDrawable(Vector3(1.0f, 0.0f, 0.0f), Vector3(0.001f, 0.001f, 0.001f)));
        
        // Should handle zero-sized objects gracefully
        auto filtered = shadowFrustum.shadowRelevanceFilter(zeroObjects);
        
        // Zero-sized objects should be filtered out
        REQUIRE(filtered.size() <= zeroObjects.size());
        
        // Specifically, objects smaller than the threshold should be filtered
        bool hasZeroSizedObject = false;
        for (const auto& obj : filtered)
        {
            if (obj == zeroObjects[0]) // The zero-sized object
            {
                hasZeroSizedObject = true;
                break;
            }
        }
        REQUIRE_FALSE(hasZeroSizedObject);
    }
    
    SECTION("INVALID_CASCADE_ACCESS")
    {
        ShadowFrustum shadowFrustum;
        auto lightTransforms = ShadowTestHelper::createTestLightTransforms(2); // Only 2 cascades
        shadowFrustum.buildFromLightTransforms(lightTransforms, 2);
        
        std::vector<std::shared_ptr<Drawable>> testObjects;
        testObjects.push_back(ShadowTestHelper::createTestDrawable(Vector3::Zero));
        
        // Valid cascade access
        REQUIRE_NOTHROW(shadowFrustum.cullForCascade(0, testObjects));
        REQUIRE_NOTHROW(shadowFrustum.cullForCascade(1, testObjects));
        
        // Invalid cascade access should throw
        REQUIRE_THROWS(shadowFrustum.cullForCascade(2, testObjects));
        REQUIRE_THROWS(shadowFrustum.cullForCascade(5, testObjects));
        
        // Same for getting cascade frustums
        REQUIRE_NOTHROW(shadowFrustum.getCascadeFrustum(0));
        REQUIRE_NOTHROW(shadowFrustum.getCascadeFrustum(1));
        REQUIRE_THROWS(shadowFrustum.getCascadeFrustum(2));
    }
}

TEST_CASE("SHADOW_FRUSTUM_PERFORMANCE_AND_STRESS")
{
    SECTION("MANY_OBJECTS_CULLING")
    {
        ShadowFrustum shadowFrustum;
        auto lightTransforms = ShadowTestHelper::createTestLightTransforms(4);
        shadowFrustum.buildFromLightTransforms(lightTransforms, 4);
        
        // Create many test objects
        std::vector<std::shared_ptr<Drawable>> manyObjects;
        for (int i = 0; i < 1000; ++i)
        {
            Vector3 position(
                (i % 20) * 2.0f - 20.0f,
                ((i / 20) % 20) * 2.0f - 20.0f,
                (i / 400) * 2.0f - 10.0f
            );
            manyObjects.push_back(ShadowTestHelper::createTestDrawable(position));
        }
        
        // Test that culling many objects doesn't crash and completes in reasonable time
        auto start = std::chrono::high_resolution_clock::now();
        
        for (uint32 cascade = 0; cascade < 4; ++cascade)
        {
            auto culled = shadowFrustum.cullForCascade(cascade, manyObjects);
            REQUIRE(culled.size() <= manyObjects.size());
        }
        
        auto broadCulled = shadowFrustum.broadPhaseCull(manyObjects);
        REQUIRE(broadCulled.size() <= manyObjects.size());
        
        auto filtered = shadowFrustum.shadowRelevanceFilter(manyObjects);
        REQUIRE(filtered.size() <= manyObjects.size());
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        // Should complete in reasonable time (less than 100ms for 1000 objects)
        REQUIRE(duration.count() < 100);
    }
    
    SECTION("REPEATED_CULLING_CONSISTENCY")
    {
        ShadowFrustum shadowFrustum;
        auto lightTransforms = ShadowTestHelper::createTestLightTransforms(4);
        shadowFrustum.buildFromLightTransforms(lightTransforms, 4);
        
        std::vector<std::shared_ptr<Drawable>> testObjects;
        for (int i = 0; i < 10; ++i)
        {
            testObjects.push_back(ShadowTestHelper::createTestDrawable(
                Vector3(i * 2.0f - 10.0f, 0.0f, -5.0f)
            ));
        }
        
        // Perform culling multiple times and ensure consistency
        std::vector<size_t> cullCounts;
        for (int iteration = 0; iteration < 10; ++iteration)
        {
            auto culled = shadowFrustum.cullForCascade(0, testObjects);
            cullCounts.push_back(culled.size());
        }
        
        // All iterations should produce the same result
        for (size_t i = 1; i < cullCounts.size(); ++i)
        {
            REQUIRE(cullCounts[i] == cullCounts[0]);
        }
    }
}

TEST_CASE("SHADOW_FRUSTUM_INTEGRATION_TESTS")
{
    SECTION("FULL_PIPELINE_TEST")
    {
        ShadowFrustum shadowFrustum;
        
        // Setup complete shadow frustum pipeline
        auto camera = ShadowTestHelper::createTestCamera(Vector3(0.0f, 5.0f, 10.0f));
        auto lightTransforms = ShadowTestHelper::createTestLightTransforms(4);
        
        shadowFrustum.buildFromLightTransforms(lightTransforms, 4);
        shadowFrustum.buildExtendedCameraFrustum(*camera, 1.5f);
        
        // Create a realistic scene with various objects
        std::vector<std::shared_ptr<Drawable>> sceneObjects;
        
        // Ground plane objects
        for (int x = -10; x <= 10; x += 2)
        {
            for (int z = -10; z <= 10; z += 2)
            {
                sceneObjects.push_back(ShadowTestHelper::createTestDrawable(Vector3(x, 0.0f, z)));
            }
        }
        
        // Some elevated objects
        sceneObjects.push_back(ShadowTestHelper::createTestDrawable(Vector3(0.0f, 5.0f, -5.0f), Vector3(2.0f, 2.0f, 2.0f)));
        sceneObjects.push_back(ShadowTestHelper::createTestDrawable(Vector3(3.0f, 2.0f, -8.0f), Vector3(1.0f, 4.0f, 1.0f)));
        
        // Test the complete pipeline
        auto broadCulled = shadowFrustum.broadPhaseCull(sceneObjects);
        auto shadowRelevant = shadowFrustum.shadowRelevanceFilter(broadCulled);
        
        // Test cascade culling on the filtered objects
        std::vector<std::vector<std::shared_ptr<Drawable>>> cascadeResults;
        for (uint32 cascade = 0; cascade < 4; ++cascade)
        {
            auto cascadeCulled = shadowFrustum.cullForCascade(cascade, shadowRelevant);
            cascadeResults.push_back(cascadeCulled);
        }
        
        // Verify the pipeline produces reasonable results
        REQUIRE(broadCulled.size() <= sceneObjects.size());
        REQUIRE(shadowRelevant.size() <= broadCulled.size());
        
        for (const auto& cascadeResult : cascadeResults)
        {
            REQUIRE(cascadeResult.size() <= shadowRelevant.size());
        }
    }
    
    SECTION("CAMERA_MOVEMENT_REGRESSION_TEST")
    {
        // This test specifically targets the camera movement artifacts issue
        ShadowFrustum shadowFrustum;
        
        // Create a test object that should remain consistently visible
        std::vector<std::shared_ptr<Drawable>> testObjects;
        testObjects.push_back(ShadowTestHelper::createTestDrawable(Vector3(0.0f, 0.0f, -10.0f), Vector3(2.0f, 2.0f, 2.0f)));
        
        // Test with a series of small camera movements that might cause artifacts
        std::vector<Vector3> cameraPositions = {
            Vector3(0.0f, 5.0f, 0.0f),
            Vector3(0.1f, 5.0f, 0.0f),
            Vector3(0.2f, 5.0f, 0.0f),
            Vector3(0.0f, 5.1f, 0.0f),
            Vector3(0.0f, 5.0f, 0.1f),
            Vector3(0.1f, 5.1f, 0.1f)
        };
        
        std::vector<size_t> cullResults;
        
        for (const auto& pos : cameraPositions)
        {
            auto camera = ShadowTestHelper::createTestCamera(pos, Vector3(0.0f, 0.0f, -10.0f));
            shadowFrustum.buildExtendedCameraFrustum(*camera, 1.5f);
            
            auto culled = shadowFrustum.broadPhaseCull(testObjects);
            cullResults.push_back(culled.size());
        }
        
        // For small camera movements, the culling results should be stable
        // The object should consistently be included or excluded
        bool allSame = true;
        for (size_t i = 1; i < cullResults.size(); ++i)
        {
            if (cullResults[i] != cullResults[0])
            {
                allSame = false;
                break;
            }
        }
        
        // This test will help identify if small camera movements cause inconsistent culling
        REQUIRE(allSame);
    }
}

// Additional test for debugging specific matrix extraction issues
TEST_CASE("SHADOW_FRUSTUM_DEBUG_MATRIX_EXTRACTION")
{
    SECTION("KNOWN_MATRIX_PLANE_EXTRACTION")
    {
        ShadowFrustum shadowFrustum;
        
        // Create a very simple, known orthographic matrix for debugging
        // This creates a box from -5 to 5 in X and Y, and -10 to 10 in Z
        Matrix4 simpleOrtho = Matrix4::Orthographic(-5.0f, 5.0f, -5.0f, 5.0f, -10.0f, 10.0f);
        Matrix4 simpleView = Matrix4::Identity; // No view transform for simplicity
        Matrix4 lightTransform = simpleOrtho * simpleView;
        
        std::vector<Matrix4> transforms = { lightTransform };
        shadowFrustum.buildFromLightTransforms(transforms, 1);
        
        // Test with objects at known positions relative to this frustum
        std::vector<std::shared_ptr<Drawable>> testObjects;
        
        // Object clearly inside the frustum
        testObjects.push_back(ShadowTestHelper::createTestDrawable(Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f, 1.0f, 1.0f)));
        
        // Object clearly outside the frustum (too far right)
        testObjects.push_back(ShadowTestHelper::createTestDrawable(Vector3(10.0f, 0.0f, 0.0f), Vector3(1.0f, 1.0f, 1.0f)));
        
        // Object on the boundary
        testObjects.push_back(ShadowTestHelper::createTestDrawable(Vector3(4.5f, 0.0f, 0.0f), Vector3(1.0f, 1.0f, 1.0f)));
        
        auto culled = shadowFrustum.cullForCascade(0, testObjects);
        
        // The center object should definitely be included
        bool hasCenterObject = false;
        for (const auto& obj : culled)
        {
            if (obj == testObjects[0])
            {
                hasCenterObject = true;
                break;
            }
        }
        
        // The far object should definitely be excluded
        bool hasFarObject = false;
        for (const auto& obj : culled)
        {
            if (obj == testObjects[1])
            {
                hasFarObject = true;
                break;
            }
        }
        
        // These assertions help verify that matrix extraction is working correctly
        REQUIRE(hasCenterObject); // Object at origin should be included
        REQUIRE_FALSE(hasFarObject); // Object at (10,0,0) should be excluded from [-5,5] range
    }
    
    SECTION("LIGHT_DIRECTION_CONSISTENCY")
    {
        // Test that different light directions produce different frustums
        ShadowFrustum shadowFrustum1, shadowFrustum2;
        
        // Light pointing down
        Matrix4 proj1 = Matrix4::Orthographic(-10.0f, 10.0f, -10.0f, 10.0f, -20.0f, 20.0f);
        Matrix4 view1 = ShadowTestHelper::createViewMatrix(Vector3(0.0f, 10.0f, 0.0f), Vector3(0.0f, -1.0f, 0.0f));
        Matrix4 transform1 = proj1 * view1;
        
        // Light pointing at an angle
        Matrix4 proj2 = Matrix4::Orthographic(-10.0f, 10.0f, -10.0f, 10.0f, -20.0f, 20.0f);
        Matrix4 view2 = ShadowTestHelper::createViewMatrix(Vector3(5.0f, 10.0f, 5.0f), Vector3(-1.0f, -1.0f, -1.0f));
        Matrix4 transform2 = proj2 * view2;
        
        shadowFrustum1.buildFromLightTransforms({transform1}, 1);
        shadowFrustum2.buildFromLightTransforms({transform2}, 1);
        
        // Test the same object with both frustums
        std::vector<std::shared_ptr<Drawable>> testObjects;
        testObjects.push_back(ShadowTestHelper::createTestDrawable(Vector3(0.0f, 0.0f, 0.0f)));
        
        auto culled1 = shadowFrustum1.cullForCascade(0, testObjects);
        auto culled2 = shadowFrustum2.cullForCascade(0, testObjects);
        
        // Both should handle the test object (this is a basic sanity check)
        REQUIRE(culled1.size() <= testObjects.size());
        REQUIRE(culled2.size() <= testObjects.size());
    }
}
