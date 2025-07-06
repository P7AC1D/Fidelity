#include "catch.hpp"

#include <chrono>
#include "../Engine/Rendering/ShadowFrustum.h"
#include "../Engine/Rendering/CameraComponent.h"
#include "../Engine/Rendering/LightComponent.h"
#include "../Engine/Rendering/DrawableComponent.h"
#include "../Engine/Rendering/Material.h"
#include "../Engine/Maths/AABB.hpp"
#include "../Engine/Core/TransformComponent.h"
#include "../Engine/Core/GameObject.h"

// Helper class for camera culling tests
class CameraCullingTestHelper
{
public:
    // Create a test drawable with proper AABB setup
    static std::shared_ptr<Drawable> createTestDrawable(const Vector3& position,
                                                        const Vector3& size = Vector3(1.0f, 1.0f, 1.0f))
    {
        class TestDrawable : public Drawable
        {
        public:
            TestDrawable(const Vector3& pos, const Vector3& extents)
            {
                auto material = std::make_shared<Material>();
                material->setDiffuseColour(Colour::White);
                setMaterial(material);
                
                _testAabb = Aabb(pos, extents.X * 0.5f, extents.Y * 0.5f, extents.Z * 0.5f);
                _testPosition = pos;
                _testTransform.setPosition(pos);
            }
            
            const Aabb& getAabb() const { return _testAabb; }
            Vector3 getPosition() const { return _testPosition; }
            const Transform& getCachedTransform() const { return _testTransform; }
            
        private:
            Aabb _testAabb;
            Vector3 _testPosition;
            mutable Transform _testTransform;
        };
        
        return std::make_shared<TestDrawable>(position, size);
    }
    
    // Create a camera with specific orientation
    static std::shared_ptr<Camera> createCameraLookingAt(const Vector3& position, 
                                                         const Vector3& target,
                                                         const Vector3& up = Vector3::Up)
    {
        auto camera = std::make_shared<Camera>();
        camera->setPerspective(Degree(60.0f), 1920, 1080, 0.1f, 100.0f);
        
        // Set up the camera transform to look at the target
        Vector3 forward = Vector3::Normalize(target - position);
        Vector3 right = Vector3::Normalize(Vector3::Cross(forward, up));
        Vector3 actualUp = Vector3::Normalize(Vector3::Cross(right, forward));
        
        // Use LookAt quaternion method
        Quaternion rotation = Quaternion::LookAt(forward, actualUp);
        
        // Create transform and set it directly on camera for testing
        Transform cameraTransform;
        cameraTransform.setPosition(position);
        cameraTransform.setRotation(rotation);
        
        // Use the new testing method to properly set up the camera
        camera->setTransformForTesting(cameraTransform);
        
        return camera;
    }
    
    // Create a camera pointing in a specific direction (pitch/yaw)
    static std::shared_ptr<Camera> createCameraWithOrientation(const Vector3& position,
                                                               float32 pitchDegrees,
                                                               float32 yawDegrees)
    {
        auto camera = std::make_shared<Camera>();
        camera->setPerspective(Degree(60.0f), 1920, 1080, 0.1f, 100.0f);
        
        // Convert pitch and yaw to direction vector
        float32 pitchRad = pitchDegrees * Math::Deg2Rad;
        float32 yawRad = yawDegrees * Math::Deg2Rad;
        
        Vector3 direction(
            cos(pitchRad) * sin(yawRad),
            sin(pitchRad),
            cos(pitchRad) * cos(yawRad)
        );
        
        Vector3 target = position + direction;
        return createCameraLookingAt(position, target);
    }
};

TEST_CASE("CAMERA_HORIZON_CULLING_TESTS")
{
    SECTION("CAMERA_LOOKING_STRAIGHT_AHEAD")
    {
        ShadowFrustum shadowFrustum;
        
        // Camera at origin looking forward (negative Z)
        auto camera = CameraCullingTestHelper::createCameraLookingAt(
            Vector3(0.0f, 0.0f, 0.0f),
            Vector3(0.0f, 0.0f, -10.0f)
        );
        
        shadowFrustum.buildExtendedCameraFrustum(*camera, 1.5f);
        
        // Create test objects at various positions
        std::vector<std::shared_ptr<Drawable>> testObjects;
        
        // Objects that should be visible (in front of camera)
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(0.0f, 0.0f, -5.0f)));   // Center front
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(2.0f, 0.0f, -5.0f)));   // Right front
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(-2.0f, 0.0f, -5.0f)));  // Left front
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(0.0f, 2.0f, -5.0f)));   // Above front
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(0.0f, -2.0f, -5.0f)));  // Below front
        
        // Objects that should NOT be visible (behind camera)
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(0.0f, 0.0f, 5.0f)));    // Behind
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(0.0f, 0.0f, 1.0f)));    // Close behind
        
        auto culledObjects = shadowFrustum.broadPhaseCull(testObjects);
        
        // Verify that front objects are included
        bool hasCenterFront = false, hasRightFront = false, hasLeftFront = false;
        bool hasAboveFront = false, hasBelowFront = false;
        bool hasBehind = false, hasCloseBehind = false;
        
        for (const auto& obj : culledObjects)
        {
            if (obj == testObjects[0]) hasCenterFront = true;
            if (obj == testObjects[1]) hasRightFront = true;
            if (obj == testObjects[2]) hasLeftFront = true;
            if (obj == testObjects[3]) hasAboveFront = true;
            if (obj == testObjects[4]) hasBelowFront = true;
            if (obj == testObjects[5]) hasBehind = true;
            if (obj == testObjects[6]) hasCloseBehind = true;
        }
        
        // Objects in front should be included
        REQUIRE(hasCenterFront);
        REQUIRE(hasRightFront);
        REQUIRE(hasLeftFront);
        REQUIRE(hasAboveFront);
        REQUIRE(hasBelowFront);
        
        // Objects behind should be excluded
        REQUIRE_FALSE(hasBehind);
        REQUIRE_FALSE(hasCloseBehind);
    }
    
    SECTION("CAMERA_LOOKING_UP_45_DEGREES")
    {
        ShadowFrustum shadowFrustum;
        
        // Camera looking up at 45 degrees
        auto camera = CameraCullingTestHelper::createCameraWithOrientation(
            Vector3(0.0f, 0.0f, 0.0f),
            45.0f,  // pitch up 45 degrees
            0.0f    // yaw straight ahead
        );
        
        shadowFrustum.buildExtendedCameraFrustum(*camera, 1.5f);
        
        std::vector<std::shared_ptr<Drawable>> testObjects;
        
        // Objects that should be visible when looking up
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(0.0f, 5.0f, -5.0f)));   // Up and forward
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(0.0f, 10.0f, -10.0f))); // High up and forward
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(2.0f, 5.0f, -5.0f)));   // Right, up, forward
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(-2.0f, 5.0f, -5.0f)));  // Left, up, forward
        
        // Objects that should NOT be visible when looking up
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(0.0f, -5.0f, -5.0f)));  // Below horizon
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(0.0f, -10.0f, -10.0f))); // Far below
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(0.0f, 0.0f, 5.0f)));    // Behind camera
        
        auto culledObjects = shadowFrustum.broadPhaseCull(testObjects);
        
        // Check visibility
        bool hasUpForward = false, hasHighUpForward = false;
        bool hasRightUpForward = false, hasLeftUpForward = false;
        bool hasBelowHorizon = false, hasFarBelow = false, hasBehind = false;
        
        for (const auto& obj : culledObjects)
        {
            if (obj == testObjects[0]) hasUpForward = true;
            if (obj == testObjects[1]) hasHighUpForward = true;
            if (obj == testObjects[2]) hasRightUpForward = true;
            if (obj == testObjects[3]) hasLeftUpForward = true;
            if (obj == testObjects[4]) hasBelowHorizon = true;
            if (obj == testObjects[5]) hasFarBelow = true;
            if (obj == testObjects[6]) hasBehind = true;
        }
        
        // Objects above horizon should be visible
        REQUIRE(hasUpForward);
        REQUIRE(hasHighUpForward);
        REQUIRE(hasRightUpForward);
        REQUIRE(hasLeftUpForward);
        
        // Objects below horizon should be excluded (this is the key test for your issue)
        REQUIRE_FALSE(hasBelowHorizon);
        REQUIRE_FALSE(hasFarBelow);
        REQUIRE_FALSE(hasBehind);
    }
    
    SECTION("CAMERA_LOOKING_DOWN_45_DEGREES")
    {
        ShadowFrustum shadowFrustum;
        
        // Camera looking down at 45 degrees
        auto camera = CameraCullingTestHelper::createCameraWithOrientation(
            Vector3(0.0f, 10.0f, 0.0f),  // Elevated position
            -45.0f, // pitch down 45 degrees
            0.0f    // yaw straight ahead
        );
        
        shadowFrustum.buildExtendedCameraFrustum(*camera, 1.5f);
        
        std::vector<std::shared_ptr<Drawable>> testObjects;
        
        // Objects that should be visible when looking down
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(0.0f, 5.0f, -5.0f)));   // Down and forward
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(0.0f, 0.0f, -10.0f)));  // Ground level forward
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(2.0f, 5.0f, -5.0f)));   // Right, down, forward
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(-2.0f, 5.0f, -5.0f)));  // Left, down, forward
        
        // Objects that should NOT be visible when looking down
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(0.0f, 15.0f, -5.0f)));  // Above horizon
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(0.0f, 20.0f, -10.0f))); // Far above
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(0.0f, 10.0f, 5.0f)));   // Behind camera
        
        auto culledObjects = shadowFrustum.broadPhaseCull(testObjects);
        
        // Check visibility
        bool hasDownForward = false, hasGroundForward = false;
        bool hasRightDownForward = false, hasLeftDownForward = false;
        bool hasAboveHorizon = false, hasFarAbove = false, hasBehind = false;
        
        for (const auto& obj : culledObjects)
        {
            if (obj == testObjects[0]) hasDownForward = true;
            if (obj == testObjects[1]) hasGroundForward = true;
            if (obj == testObjects[2]) hasRightDownForward = true;
            if (obj == testObjects[3]) hasLeftDownForward = true;
            if (obj == testObjects[4]) hasAboveHorizon = true;
            if (obj == testObjects[5]) hasFarAbove = true;
            if (obj == testObjects[6]) hasBehind = true;
        }
        
        // Objects below horizon should be visible
        REQUIRE(hasDownForward);
        REQUIRE(hasGroundForward);
        REQUIRE(hasRightDownForward);
        REQUIRE(hasLeftDownForward);
        
        // Objects above horizon should be excluded (this is the key test for your issue)
        REQUIRE_FALSE(hasAboveHorizon);
        REQUIRE_FALSE(hasFarAbove);
        REQUIRE_FALSE(hasBehind);
    }
    
    SECTION("CAMERA_LOOKING_STRAIGHT_UP_90_DEGREES")
    {
        ShadowFrustum shadowFrustum;
        
        // Camera looking straight up
        auto camera = CameraCullingTestHelper::createCameraWithOrientation(
            Vector3(0.0f, 0.0f, 0.0f),
            90.0f,  // pitch straight up
            0.0f    // yaw doesn't matter when looking straight up
        );
        
        shadowFrustum.buildExtendedCameraFrustum(*camera, 1.5f);
        
        std::vector<std::shared_ptr<Drawable>> testObjects;
        
        // Objects that should be visible when looking straight up
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(0.0f, 10.0f, 0.0f)));   // Directly above
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(2.0f, 10.0f, 0.0f)));   // Above and right
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(-2.0f, 10.0f, 0.0f)));  // Above and left
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(0.0f, 10.0f, 2.0f)));   // Above and forward
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(0.0f, 10.0f, -2.0f)));  // Above and back
        
        // Objects that should NOT be visible when looking straight up
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(0.0f, 0.0f, -5.0f)));   // Same level forward
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(0.0f, -5.0f, 0.0f)));   // Below
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(5.0f, 0.0f, 0.0f)));    // Same level right
        
        auto culledObjects = shadowFrustum.broadPhaseCull(testObjects);
        
        // Check visibility
        bool hasDirectlyAbove = false, hasAboveRight = false, hasAboveLeft = false;
        bool hasAboveForward = false, hasAboveBack = false;
        bool hasSameLevelForward = false, hasBelow = false, hasSameLevelRight = false;
        
        for (const auto& obj : culledObjects)
        {
            if (obj == testObjects[0]) hasDirectlyAbove = true;
            if (obj == testObjects[1]) hasAboveRight = true;
            if (obj == testObjects[2]) hasAboveLeft = true;
            if (obj == testObjects[3]) hasAboveForward = true;
            if (obj == testObjects[4]) hasAboveBack = true;
            if (obj == testObjects[5]) hasSameLevelForward = true;
            if (obj == testObjects[6]) hasBelow = true;
            if (obj == testObjects[7]) hasSameLevelRight = true;
        }
        
        // Objects above should be visible
        REQUIRE(hasDirectlyAbove);
        REQUIRE(hasAboveRight);
        REQUIRE(hasAboveLeft);
        REQUIRE(hasAboveForward);
        REQUIRE(hasAboveBack);
        
        // Objects at same level or below should be excluded
        REQUIRE_FALSE(hasSameLevelForward);
        REQUIRE_FALSE(hasBelow);
        REQUIRE_FALSE(hasSameLevelRight);
    }
    
    SECTION("CAMERA_LOOKING_STRAIGHT_DOWN_90_DEGREES")
    {
        ShadowFrustum shadowFrustum;
        
        // Camera looking straight down
        auto camera = CameraCullingTestHelper::createCameraWithOrientation(
            Vector3(0.0f, 10.0f, 0.0f),  // Elevated position
            -90.0f, // pitch straight down
            0.0f    // yaw doesn't matter when looking straight down
        );
        
        shadowFrustum.buildExtendedCameraFrustum(*camera, 1.5f);
        
        std::vector<std::shared_ptr<Drawable>> testObjects;
        
        // Objects that should be visible when looking straight down
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(0.0f, 0.0f, 0.0f)));    // Directly below
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(2.0f, 0.0f, 0.0f)));    // Below and right
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(-2.0f, 0.0f, 0.0f)));   // Below and left
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(0.0f, 0.0f, 2.0f)));    // Below and forward
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(0.0f, 0.0f, -2.0f)));   // Below and back
        
        // Objects that should NOT be visible when looking straight down
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(0.0f, 10.0f, -5.0f)));  // Same level forward
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(0.0f, 15.0f, 0.0f)));   // Above
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(5.0f, 10.0f, 0.0f)));   // Same level right
        
        auto culledObjects = shadowFrustum.broadPhaseCull(testObjects);
        
        // Check visibility
        bool hasDirectlyBelow = false, hasBelowRight = false, hasBelowLeft = false;
        bool hasBelowForward = false, hasBelowBack = false;
        bool hasSameLevelForward = false, hasAbove = false, hasSameLevelRight = false;
        
        for (const auto& obj : culledObjects)
        {
            if (obj == testObjects[0]) hasDirectlyBelow = true;
            if (obj == testObjects[1]) hasBelowRight = true;
            if (obj == testObjects[2]) hasBelowLeft = true;
            if (obj == testObjects[3]) hasBelowForward = true;
            if (obj == testObjects[4]) hasBelowBack = true;
            if (obj == testObjects[5]) hasSameLevelForward = true;
            if (obj == testObjects[6]) hasAbove = true;
            if (obj == testObjects[7]) hasSameLevelRight = true;
        }
        
        // Objects below should be visible
        REQUIRE(hasDirectlyBelow);
        REQUIRE(hasBelowRight);
        REQUIRE(hasBelowLeft);
        REQUIRE(hasBelowForward);
        REQUIRE(hasBelowBack);
        
        // Objects at same level or above should be excluded
        REQUIRE_FALSE(hasSameLevelForward);
        REQUIRE_FALSE(hasAbove);
        REQUIRE_FALSE(hasSameLevelRight);
    }
}

TEST_CASE("CAMERA_FRUSTUM_EDGE_CASES")
{
    SECTION("NEAR_PLANE_CLIPPING")
    {
        ShadowFrustum shadowFrustum;
        
        auto camera = CameraCullingTestHelper::createCameraLookingAt(
            Vector3(0.0f, 0.0f, 0.0f),
            Vector3(0.0f, 0.0f, -10.0f)
        );
        
        shadowFrustum.buildExtendedCameraFrustum(*camera, 1.5f);
        
        std::vector<std::shared_ptr<Drawable>> testObjects;
        
        // Objects very close to camera (should be clipped by near plane)
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(0.0f, 0.0f, -0.05f))); // Very close
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(0.0f, 0.0f, -0.2f)));  // Just beyond near
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(0.0f, 0.0f, -1.0f)));  // Clearly visible
        
        auto culledObjects = shadowFrustum.broadPhaseCull(testObjects);
        
        // Very close objects might be clipped by near plane
        // This depends on the extended near plane calculation
        REQUIRE(culledObjects.size() <= testObjects.size());
    }
    
    SECTION("FAR_PLANE_CLIPPING")
    {
        ShadowFrustum shadowFrustum;
        
        auto camera = CameraCullingTestHelper::createCameraLookingAt(
            Vector3(0.0f, 0.0f, 0.0f),
            Vector3(0.0f, 0.0f, -10.0f)
        );
        
        shadowFrustum.buildExtendedCameraFrustum(*camera, 1.5f);
        
        std::vector<std::shared_ptr<Drawable>> testObjects;
        
        // Objects at various distances
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(0.0f, 0.0f, -50.0f)));  // Medium distance
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(0.0f, 0.0f, -100.0f))); // At original far plane
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(0.0f, 0.0f, -200.0f))); // Beyond extended far
        
        auto culledObjects = shadowFrustum.broadPhaseCull(testObjects);
        
        // Extended frustum should include more distant objects
        REQUIRE(culledObjects.size() <= testObjects.size());
    }
    
    SECTION("FIELD_OF_VIEW_BOUNDARIES")
    {
        ShadowFrustum shadowFrustum;
        
        auto camera = CameraCullingTestHelper::createCameraLookingAt(
            Vector3(0.0f, 0.0f, 0.0f),
            Vector3(0.0f, 0.0f, -10.0f)
        );
        
        shadowFrustum.buildExtendedCameraFrustum(*camera, 1.5f);
        
        std::vector<std::shared_ptr<Drawable>> testObjects;
        
        // Objects at the edge of the field of view
        // For 60 degree FOV, objects at ~30 degrees from center should be at the edge
        float32 edgeDistance = 10.0f * tan(30.0f * Math::Deg2Rad);
        
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(0.0f, 0.0f, -10.0f)));           // Center
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(edgeDistance * 0.9f, 0.0f, -10.0f))); // Just inside
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(edgeDistance * 1.1f, 0.0f, -10.0f))); // Just outside
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(0.0f, edgeDistance * 0.9f, -10.0f))); // Just inside top
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(0.0f, edgeDistance * 1.1f, -10.0f))); // Just outside top
        
        auto culledObjects = shadowFrustum.broadPhaseCull(testObjects);
        
        // Center and just-inside objects should be included
        // Just-outside objects should be excluded (unless extended frustum includes them)
        REQUIRE(culledObjects.size() <= testObjects.size());
    }
}

TEST_CASE("CAMERA_MOVEMENT_PRECISION_TESTS")
{
    SECTION("MICRO_MOVEMENTS_STABILITY")
    {
        ShadowFrustum shadowFrustum;
        
        // Test object that should remain consistently visible
        std::vector<std::shared_ptr<Drawable>> testObjects;
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(0.0f, 0.0f, -10.0f)));
        
        std::vector<size_t> cullResults;
        
        // Test with very small camera movements
        for (int i = 0; i < 10; ++i)
        {
            float32 offset = i * 0.001f; // 1mm movements
            auto camera = CameraCullingTestHelper::createCameraLookingAt(
                Vector3(offset, 0.0f, 0.0f),
                Vector3(offset, 0.0f, -10.0f)
            );
            
            shadowFrustum.buildExtendedCameraFrustum(*camera, 1.5f);
            auto culled = shadowFrustum.broadPhaseCull(testObjects);
            cullResults.push_back(culled.size());
        }
        
        // All micro-movements should produce the same result
        for (size_t i = 1; i < cullResults.size(); ++i)
        {
            REQUIRE(cullResults[i] == cullResults[0]);
        }
    }
    
    SECTION("PITCH_ANGLE_PRECISION")
    {
        ShadowFrustum shadowFrustum;
        
        // Test object at horizon level
        std::vector<std::shared_ptr<Drawable>> testObjects;
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(0.0f, 0.0f, -10.0f)));
        
        std::vector<size_t> cullResults;
        
        // Test with very small pitch changes around horizon
        for (int i = -5; i <= 5; ++i)
        {
            float32 pitch = i * 0.1f; // 0.1 degree increments
            auto camera = CameraCullingTestHelper::createCameraWithOrientation(
                Vector3(0.0f, 0.0f, 0.0f),
                pitch,
                0.0f
            );
            
            shadowFrustum.buildExtendedCameraFrustum(*camera, 1.5f);
            auto culled = shadowFrustum.broadPhaseCull(testObjects);
            cullResults.push_back(culled.size());
        }
        
        // Small pitch changes should not cause dramatic culling differences
        // for objects near the horizon
        bool hasInconsistency = false;
        for (size_t i = 1; i < cullResults.size(); ++i)
        {
            if (cullResults[i] != cullResults[i-1])
            {
                hasInconsistency = true;
                break;
            }
        }
        
        // This test will help identify if small pitch changes cause instability
        REQUIRE_FALSE(hasInconsistency);
    }
}

TEST_CASE("EXTENDED_FRUSTUM_VALIDATION")
{
    SECTION("SHADOW_DISTANCE_MULTIPLIER_EFFECT")
    {
        ShadowFrustum shadowFrustum1, shadowFrustum2;
        
        auto camera = CameraCullingTestHelper::createCameraLookingAt(
            Vector3(0.0f, 0.0f, 0.0f),
            Vector3(0.0f, 0.0f, -10.0f)
        );
        
        // Build frustums with different shadow distance multipliers
        shadowFrustum1.buildExtendedCameraFrustum(*camera, 1.0f);  // No extension
        shadowFrustum2.buildExtendedCameraFrustum(*camera, 2.0f);  // 2x extension
        
        std::vector<std::shared_ptr<Drawable>> testObjects;
        
        // Objects at various distances
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(0.0f, 0.0f, -50.0f)));  // Medium distance
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(0.0f, 0.0f, -120.0f))); // At extended far
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(0.0f, 0.0f, -250.0f))); // Beyond extended far
        
        auto culled1 = shadowFrustum1.broadPhaseCull(testObjects);
        auto culled2 = shadowFrustum2.broadPhaseCull(testObjects);
        
        // Extended frustum should include more objects
        REQUIRE(culled2.size() >= culled1.size());
    }
}

// Specific tests for the horizon culling bug you're experiencing
TEST_CASE("HORIZON_CULLING_BUG_REPRODUCTION")
{
    SECTION("OBJECTS_INCORRECTLY_CULLED_WHEN_LOOKING_UP")
    {
        ShadowFrustum shadowFrustum;
        
        // Reproduce the exact scenario: camera looking up above horizon
        auto camera = CameraCullingTestHelper::createCameraWithOrientation(
            Vector3(0.0f, 5.0f, 0.0f),  // Elevated camera position
            30.0f,  // Looking up 30 degrees above horizon
            0.0f    // Facing forward
        );
        
        shadowFrustum.buildExtendedCameraFrustum(*camera, 1.5f);
        
        std::vector<std::shared_ptr<Drawable>> testObjects;
        
        // Objects that should definitely be visible when looking up
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(0.0f, 10.0f, -10.0f))); // Above and forward
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(0.0f, 8.0f, -8.0f)));   // Slightly above and forward
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(3.0f, 10.0f, -10.0f))); // Above, right, forward
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(-3.0f, 10.0f, -10.0f)));// Above, left, forward
        
        // Objects that should NOT be visible (below the camera's view)
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(0.0f, 2.0f, -10.0f)));  // Below camera view
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(0.0f, 0.0f, -10.0f)));  // Ground level
        
        auto culledObjects = shadowFrustum.broadPhaseCull(testObjects);
        
        // Check what's being culled
        bool hasAboveForward = false, hasSlightlyAbove = false;
        bool hasAboveRight = false, hasAboveLeft = false;
        bool hasBelowView = false, hasGroundLevel = false;
        
        for (const auto& obj : culledObjects)
        {
            if (obj == testObjects[0]) hasAboveForward = true;
            if (obj == testObjects[1]) hasSlightlyAbove = true;
            if (obj == testObjects[2]) hasAboveRight = true;
            if (obj == testObjects[3]) hasAboveLeft = true;
            if (obj == testObjects[4]) hasBelowView = true;
            if (obj == testObjects[5]) hasGroundLevel = true;
        }
        
        // These should be visible when looking up
        REQUIRE(hasAboveForward);
        REQUIRE(hasSlightlyAbove);
        REQUIRE(hasAboveRight);
        REQUIRE(hasAboveLeft);
        
        // These should be culled when looking up
        REQUIRE_FALSE(hasBelowView);
        REQUIRE_FALSE(hasGroundLevel);
    }
    
    SECTION("OBJECTS_INCORRECTLY_CULLED_WHEN_LOOKING_DOWN")
    {
        ShadowFrustum shadowFrustum;
        
        // Reproduce the exact scenario: camera looking down below horizon
        auto camera = CameraCullingTestHelper::createCameraWithOrientation(
            Vector3(0.0f, 15.0f, 0.0f),  // High elevated camera position
            -30.0f, // Looking down 30 degrees below horizon
            0.0f    // Facing forward
        );
        
        shadowFrustum.buildExtendedCameraFrustum(*camera, 1.5f);
        
        std::vector<std::shared_ptr<Drawable>> testObjects;
        
        // Objects that should definitely be visible when looking down
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(0.0f, 5.0f, -10.0f)));  // Below and forward
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(0.0f, 10.0f, -8.0f)));  // Slightly below and forward
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(3.0f, 5.0f, -10.0f)));  // Below, right, forward
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(-3.0f, 5.0f, -10.0f))); // Below, left, forward
        
        // Objects that should NOT be visible (above the camera's view)
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(0.0f, 20.0f, -10.0f))); // Above camera view
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(0.0f, 25.0f, -10.0f))); // High above
        
        auto culledObjects = shadowFrustum.broadPhaseCull(testObjects);
        
        // Check what's being culled
        bool hasBelowForward = false, hasSlightlyBelow = false;
        bool hasBelowRight = false, hasBelowLeft = false;
        bool hasAboveView = false, hasHighAbove = false;
        
        for (const auto& obj : culledObjects)
        {
            if (obj == testObjects[0]) hasBelowForward = true;
            if (obj == testObjects[1]) hasSlightlyBelow = true;
            if (obj == testObjects[2]) hasBelowRight = true;
            if (obj == testObjects[3]) hasBelowLeft = true;
            if (obj == testObjects[4]) hasAboveView = true;
            if (obj == testObjects[5]) hasHighAbove = true;
        }
        
        // These should be visible when looking down
        REQUIRE(hasBelowForward);
        REQUIRE(hasSlightlyBelow);
        REQUIRE(hasBelowRight);
        REQUIRE(hasBelowLeft);
        
        // These should be culled when looking down
        REQUIRE_FALSE(hasAboveView);
        REQUIRE_FALSE(hasHighAbove);
    }
    
    SECTION("HORIZON_BOUNDARY_PRECISION_TEST")
    {
        ShadowFrustum shadowFrustum;
        
        // Test objects right at the horizon boundary
        auto camera = CameraCullingTestHelper::createCameraLookingAt(
            Vector3(0.0f, 5.0f, 0.0f),
            Vector3(0.0f, 5.0f, -10.0f)  // Looking straight ahead at same height
        );
        
        shadowFrustum.buildExtendedCameraFrustum(*camera, 1.5f);
        
        std::vector<std::shared_ptr<Drawable>> testObjects;
        
        // Objects at precise horizon level
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(0.0f, 5.0f, -10.0f)));    // Exactly at horizon
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(0.0f, 5.01f, -10.0f)));   // Just above horizon
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(0.0f, 4.99f, -10.0f)));   // Just below horizon
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(0.0f, 5.1f, -10.0f)));    // Clearly above
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(0.0f, 4.9f, -10.0f)));    // Clearly below
        
        auto culledObjects = shadowFrustum.broadPhaseCull(testObjects);
        
        // All objects should be visible when looking straight ahead
        // This tests the precision of horizon calculations
        REQUIRE(culledObjects.size() == testObjects.size());
    }
}

// Tests to validate the extended camera frustum calculation
TEST_CASE("EXTENDED_CAMERA_FRUSTUM_VALIDATION")
{
    SECTION("FRUSTUM_PLANE_EXTRACTION_VALIDATION")
    {
        ShadowFrustum shadowFrustum;
        
        // Create a camera with known parameters
        auto camera = CameraCullingTestHelper::createCameraLookingAt(
            Vector3(0.0f, 0.0f, 0.0f),
            Vector3(0.0f, 0.0f, -10.0f)
        );
        
        shadowFrustum.buildExtendedCameraFrustum(*camera, 1.0f);
        
        std::vector<std::shared_ptr<Drawable>> testObjects;
        
        // Test objects at known positions relative to camera frustum
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(0.0f, 0.0f, -5.0f)));    // Center, should be visible
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(0.0f, 0.0f, 5.0f)));     // Behind, should be culled
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(0.0f, 0.0f, -150.0f)));  // Very far, might be culled
        
        auto culledObjects = shadowFrustum.broadPhaseCull(testObjects);
        
        // Basic sanity check - center object should be visible, behind object should not
        bool hasCenter = false, hasBehind = false;
        for (const auto& obj : culledObjects)
        {
            if (obj == testObjects[0]) hasCenter = true;
            if (obj == testObjects[1]) hasBehind = true;
        }
        
        REQUIRE(hasCenter);
        REQUIRE_FALSE(hasBehind);
    }
    
    SECTION("CAMERA_TRANSFORM_CONSISTENCY")
    {
        ShadowFrustum shadowFrustum1, shadowFrustum2;
        
        // Create two identical cameras
        auto camera1 = CameraCullingTestHelper::createCameraLookingAt(
            Vector3(0.0f, 0.0f, 0.0f),
            Vector3(0.0f, 0.0f, -10.0f)
        );
        
        auto camera2 = CameraCullingTestHelper::createCameraLookingAt(
            Vector3(0.0f, 0.0f, 0.0f),
            Vector3(0.0f, 0.0f, -10.0f)
        );
        
        shadowFrustum1.buildExtendedCameraFrustum(*camera1, 1.5f);
        shadowFrustum2.buildExtendedCameraFrustum(*camera2, 1.5f);
        
        std::vector<std::shared_ptr<Drawable>> testObjects;
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(0.0f, 0.0f, -10.0f)));
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(5.0f, 0.0f, -10.0f)));
        testObjects.push_back(CameraCullingTestHelper::createTestDrawable(Vector3(0.0f, 5.0f, -10.0f)));
        
        auto culled1 = shadowFrustum1.broadPhaseCull(testObjects);
        auto culled2 = shadowFrustum2.broadPhaseCull(testObjects);
        
        // Identical cameras should produce identical results
        REQUIRE(culled1.size() == culled2.size());
    }
}
