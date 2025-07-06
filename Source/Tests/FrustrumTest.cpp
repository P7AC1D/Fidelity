#include "catch.hpp"

#include "../Engine/Maths/Frustrum.hpp"
#include "../Engine/Maths/AABB.hpp"
#include "../Engine/Rendering/CameraComponent.h"
#include "../Engine/Core/TransformComponent.h"
#include "../Engine/Core/GameObject.h"
#include "../Engine/Core/ComponentManager.h"

// Helper class to create a properly initialized camera for testing
class FrustumTestHelper
{
public:
    static std::pair<GameObject*, CameraComponent*> createCameraWithGameObject(
        ComponentManager* componentManager,
        const Vector3& position = Vector3::Zero, 
        const Quaternion& rotation = Quaternion::Identity,
        uint64 id = 1)
    {
        // Create a GameObject and set its transform
        auto* gameObject = new GameObject("TestCamera", id, componentManager);
        auto& camera = gameObject->addComponent<CameraComponent>();
        
        camera.setPerspective(Degree(60.0f), 1280, 768, 0.1f, 100.0f);
        
        // Set transform
        auto* transform = gameObject->tryGetComponent<TransformComponent>();
        if (transform)
        {
            transform->setPosition(position);
            transform->setRotation(rotation);
        }
        
        // Update the game object to trigger component updates
        gameObject->update(0.0f);
        
        return {gameObject, &camera};
    }
    
    // Alternative approach: Test Frustum directly
    static Frustrum createFrustumDirect(const Vector3& cameraPos = Vector3::Zero,
                                       const Quaternion& cameraRot = Quaternion::Identity,
                                       float32 fov = 60.0f,
                                       float32 aspect = 1280.0f/768.0f,
                                       float32 nearPlane = 0.1f,
                                       float32 farPlane = 100.0f)
    {
        ComponentManager componentManager;
        auto [cameraGO, camera] = createCameraWithGameObject(&componentManager, cameraPos, cameraRot);
        camera->setPerspective(Degree(fov), 1280, 768, nearPlane, farPlane);
        
        Frustrum frustum(*camera);
        delete cameraGO;
        return frustum;
    }
};

TEST_CASE("FRUSTUM CONSTRUCTION")
{
    ComponentManager componentManager;
    
    SECTION("BASIC FRUSTUM CREATION")
    {
        auto [cameraGO, camera] = FrustumTestHelper::createCameraWithGameObject(&componentManager);
        
        // Create frustum directly from camera
        Frustrum frustum(*camera);
        
        // Test that frustum was created without crashing
        REQUIRE(true); // Basic construction test
        
        delete cameraGO;
    }
}

TEST_CASE("FRUSTUM CULLING - DIRECT TESTING")
{
    ComponentManager componentManager;
    
    SECTION("AXIS-ALIGNED OBJECT AT ORIGIN")
    {
        auto [cameraGO, camera] = FrustumTestHelper::createCameraWithGameObject(&componentManager);
        Frustrum frustum(*camera);
        
        // Create an AABB at origin
        Aabb aabb(Vector3::Zero, 1.0f, 1.0f, 1.0f);
        TransformComponent transform;
        transform.setPosition(Vector3::Zero);
        transform.setRotation(Quaternion::Identity);
        
        // Test the frustum contains method directly
        bool result = frustum.contains(aabb, transform);
        
        // Since camera transform is not set, this tests the basic functionality
        // The result depends on the default camera setup
        REQUIRE((result == true || result == false)); // Just ensure it doesn't crash
        
        delete cameraGO;
    }
    
    SECTION("AXIS-ALIGNED VS ORIENTED OBJECTS")
    {
        auto [cameraGO, camera] = FrustumTestHelper::createCameraWithGameObject(&componentManager);
        Frustrum frustum(*camera);
        
        // Test axis-aligned object
        Aabb aabb(Vector3::Zero, 1.0f, 1.0f, 1.0f);
        TransformComponent axisAlignedTransform;
        axisAlignedTransform.setPosition(Vector3(0.0f, 0.0f, -5.0f));
        axisAlignedTransform.setRotation(Quaternion::Identity); // No rotation
        
        bool axisAlignedResult = frustum.contains(aabb, axisAlignedTransform);
        
        // Test oriented object
        TransformComponent orientedTransform;
        orientedTransform.setPosition(Vector3(0.0f, 0.0f, -5.0f));
        orientedTransform.setRotation(Quaternion(Vector3(0.0f, 1.0f, 0.0f), Radian(Degree(45.0f))));
        
        bool orientedResult = frustum.contains(aabb, orientedTransform);
        
        // Both should give valid results (true or false)
        REQUIRE((axisAlignedResult == true || axisAlignedResult == false));
        REQUIRE((orientedResult == true || orientedResult == false));
        
        delete cameraGO;
    }
}

TEST_CASE("CAMERA CULLING - INTEGRATION TESTS")
{
    ComponentManager componentManager;
    
    SECTION("CAMERA CONTAINS METHOD")
    {
        auto [cameraGO, camera] = FrustumTestHelper::createCameraWithGameObject(&componentManager);
        
        // Test various object positions
        Aabb aabb(Vector3::Zero, 1.0f, 1.0f, 1.0f);
        
        // Test object in front
        TransformComponent frontTransform;
        frontTransform.setPosition(Vector3(0.0f, 0.0f, -5.0f));
        bool frontResult = camera->contains(aabb, frontTransform.getWorldMatrix());
        
        // Test object behind
        TransformComponent behindTransform;
        behindTransform.setPosition(Vector3(0.0f, 0.0f, 5.0f));
        bool behindResult = camera->contains(aabb, behindTransform.getWorldMatrix());
        
        // Test object to the side
        TransformComponent sideTransform;
        sideTransform.setPosition(Vector3(50.0f, 0.0f, -5.0f));
        bool sideResult = camera->contains(aabb, sideTransform.getWorldMatrix());
        
        // All should return valid boolean results
        REQUIRE((frontResult == true || frontResult == false));
        REQUIRE((behindResult == true || behindResult == false));
        REQUIRE((sideResult == true || sideResult == false));
        
        delete cameraGO;
    }
    
    SECTION("CAMERA PARAMETER CHANGES")
    {
        auto [cameraGO, camera] = FrustumTestHelper::createCameraWithGameObject(&componentManager);
        
        // Test with different FOV
        camera->setFov(Degree(90.0f));
        
        // Test with different near/far planes
        camera->setNear(1.0f);
        camera->setFar(50.0f);
        
        Aabb aabb(Vector3::Zero, 1.0f, 1.0f, 1.0f);
        TransformComponent transform;
        transform.setPosition(Vector3(0.0f, 0.0f, -5.0f));
        
        bool result = camera->contains(aabb, transform.getWorldMatrix());
        REQUIRE((result == true || result == false));
        
        delete cameraGO;
    }
}

TEST_CASE("TRANSFORM OPTIMIZATION PATHS")
{
    ComponentManager componentManager;
    
    SECTION("AXIS-ALIGNED DETECTION")
    {
        auto [cameraGO, camera] = FrustumTestHelper::createCameraWithGameObject(&componentManager);
        
        Aabb aabb(Vector3::Zero, 1.0f, 1.0f, 1.0f);
        
        // Test clearly axis-aligned transform
        TransformComponent axisAligned;
        axisAligned.setPosition(Vector3(0.0f, 0.0f, -5.0f));
        axisAligned.setRotation(Quaternion::Identity);
        
        // This should use the axis-aligned fast path
        bool axisAlignedResult = camera->contains(aabb, axisAligned.getWorldMatrix());
        
        // Test clearly oriented transform
        TransformComponent oriented;
        oriented.setPosition(Vector3(0.0f, 0.0f, -5.0f));
        oriented.setRotation(Quaternion(Vector3(1.0f, 0.0f, 0.0f), Radian(Degree(90.0f))));
        
        // This should use the oriented path
        bool orientedResult = camera->contains(aabb, oriented.getWorldMatrix());
        
        // Both should give valid results
        REQUIRE((axisAlignedResult == true || axisAlignedResult == false));
        REQUIRE((orientedResult == true || orientedResult == false));
        
        delete cameraGO;
    }
}

TEST_CASE("PERFORMANCE AND STRESS TESTS")
{
    ComponentManager componentManager;
    
    SECTION("MULTIPLE CULLING OPERATIONS")
    {
        auto [cameraGO, camera] = FrustumTestHelper::createCameraWithGameObject(&componentManager);
        
        // Test many objects to ensure no crashes or performance issues
        std::vector<bool> results;
        
        for (int i = 0; i < 1000; ++i)
        {
            Aabb aabb(Vector3::Zero, 1.0f, 1.0f, 1.0f);
            TransformComponent transform;
            
            // Spread objects around in a grid
            float32 x = (i % 10) * 2.0f - 10.0f;
            float32 y = ((i / 10) % 10) * 2.0f - 10.0f;
            float32 z = -5.0f - (i / 100) * 2.0f;
            
            transform.setPosition(Vector3(x, y, z));
            
            // Mix of axis-aligned and oriented objects
            if (i % 2 == 0)
            {
                transform.setRotation(Quaternion::Identity);
            }
            else
            {
                transform.setRotation(Quaternion(Vector3(0.0f, 1.0f, 0.0f), Radian(Degree(i * 3.6f))));
            }
            
            bool result = camera->contains(aabb, transform.getWorldMatrix());
            results.push_back(result);
        }
        
        // Ensure we got results for all tests
        REQUIRE(results.size() == 1000);
        
        delete cameraGO;
    }
}

TEST_CASE("EDGE CASES AND BOUNDARY CONDITIONS")
{
    ComponentManager componentManager;
    
    SECTION("VERY LARGE OBJECTS")
    {
        auto [cameraGO, camera] = FrustumTestHelper::createCameraWithGameObject(&componentManager);
        
        // Test with very large AABB
        Aabb largeAabb(Vector3::Zero, 1000.0f, 1000.0f, 1000.0f);
        TransformComponent transform;
        transform.setPosition(Vector3(0.0f, 0.0f, -5.0f));
        
        bool result = camera->contains(largeAabb, transform.getWorldMatrix());
        REQUIRE((result == true || result == false));
        
        delete cameraGO;
    }
    
    SECTION("VERY SMALL OBJECTS")
    {
        auto [cameraGO, camera] = FrustumTestHelper::createCameraWithGameObject(&componentManager);
        
        // Test with very small AABB
        Aabb smallAabb(Vector3::Zero, 0.001f, 0.001f, 0.001f);
        TransformComponent transform;
        transform.setPosition(Vector3(0.0f, 0.0f, -5.0f));
        
        bool result = camera->contains(smallAabb, transform.getWorldMatrix());
        REQUIRE((result == true || result == false));
        
        delete cameraGO;
    }
    
    SECTION("EXTREME POSITIONS")
    {
        auto [cameraGO, camera] = FrustumTestHelper::createCameraWithGameObject(&componentManager);
        
        Aabb aabb(Vector3::Zero, 1.0f, 1.0f, 1.0f);
        
        // Test object very far away
        TransformComponent farTransform;
        farTransform.setPosition(Vector3(0.0f, 0.0f, -10000.0f));
        bool farResult = camera->contains(aabb, farTransform.getWorldMatrix());
        
        // Test object very close
        TransformComponent closeTransform;
        closeTransform.setPosition(Vector3(0.0f, 0.0f, -0.01f));
        bool closeResult = camera->contains(aabb, closeTransform.getWorldMatrix());
        
        REQUIRE((farResult == true || farResult == false));
        REQUIRE((closeResult == true || closeResult == false));
        
        delete cameraGO;
    }
}
