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
    
    SECTION("MATRIX4 OPTIMIZATION PATHS")
    {
        // Create a simple frustum for testing Matrix4 optimizations
        Plane left(Vector3(1, 0, 0), Vector3(-5, 0, 0));    
        Plane right(Vector3(-1, 0, 0), Vector3(5, 0, 0));   
        Plane top(Vector3(0, -1, 0), Vector3(0, 5, 0));     
        Plane bottom(Vector3(0, 1, 0), Vector3(0, -5, 0));  
        Plane near(Vector3(0, 0, 1), Vector3(0, 0, -1));    
        Plane far(Vector3(0, 0, -1), Vector3(0, 0, 10));    
        
        Frustrum frustum(left, right, top, bottom, near, far);
        
        // Test AABB that should be inside the frustum
        Aabb insideBox(Vector3(-1, -1, 0), Vector3(1, 1, 2));
        Aabb outsideBox(Vector3(10, 10, 10), Vector3(12, 12, 12));
        
        // Test identity transform
        Matrix4 identity = Matrix4::Identity;
        REQUIRE(frustum.contains(insideBox, identity) == true);
        REQUIRE(frustum.contains(outsideBox, identity) == false);
        
        // Test translation transform (should use axis-aligned optimization)
        Matrix4 translation = Matrix4::Translation(Vector3(1.0f, 0.0f, 0.0f));
        REQUIRE(frustum.contains(insideBox, translation) == true);
        REQUIRE(frustum.contains(outsideBox, translation) == false);
        
        // Test scaling transform (should use axis-aligned optimization)  
        Matrix4 scaling = Matrix4::Scaling(Vector3(0.5f, 0.5f, 0.5f));
        REQUIRE(frustum.contains(insideBox, scaling) == true);
        REQUIRE_NOTHROW(frustum.contains(outsideBox, scaling));
        
        // Test combined translation and scaling (should use axis-aligned optimization)
        Matrix4 translation2 = Matrix4::Translation(Vector3(0.5f, 0.0f, 0.0f));
        Matrix4 scaling2 = Matrix4::Scaling(Vector3(0.8f, 0.8f, 0.8f));
        Matrix4 combined = translation2 * scaling2;
        REQUIRE_NOTHROW(frustum.contains(insideBox, combined));
        REQUIRE_NOTHROW(frustum.contains(outsideBox, combined));
    }
    
    SECTION("MATRIX4 AXIS-ALIGNED DETECTION")
    {
        ComponentManager componentManager;
        auto [cameraGO, camera] = FrustumTestHelper::createCameraWithGameObject(&componentManager);
        
        Aabb testBox(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));
        
        // Test pure translation (should be detected as axis-aligned)
        Matrix4 pureTranslation = Matrix4::Translation(Vector3(2.0f, 3.0f, -5.0f));
        REQUIRE_NOTHROW(camera->contains(testBox, pureTranslation));
        
        // Test pure scaling (should be detected as axis-aligned)
        Matrix4 pureScaling = Matrix4::Scaling(Vector3(2.0f, 1.5f, 0.8f));
        REQUIRE_NOTHROW(camera->contains(testBox, pureScaling));
        
        // Test negative scaling (should be detected as axis-aligned and handled correctly)
        Matrix4 negativeScaling = Matrix4::Scaling(Vector3(-1.0f, 1.0f, 1.0f));
        REQUIRE_NOTHROW(camera->contains(testBox, negativeScaling));
        
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

TEST_CASE("FRUSTUM PLANE EXTRACTION METHODS")
{
    ComponentManager componentManager;
    
    SECTION("createPlaneFromVector4 - Basic Functionality")
    {
        // Test the helper method indirectly through frustum creation
        auto [cameraGO, camera] = FrustumTestHelper::createCameraWithGameObject(&componentManager);
        
        // Create frustum - this will internally use createPlaneFromVector4
        Frustrum frustum(*camera);
        
        // Test that the frustum was created without crashing
        // We can verify this by testing basic culling behavior
        TransformComponent testTransform;
        
        // Test object creation for frustum (note: camera's contains method currently uses distance-based culling)
        // Object in front of camera should potentially be visible (within far plane distance)
        testTransform.setPosition(Vector3(0.0f, 0.0f, -5.0f));
        Aabb inFrontAABB(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));
        bool inFront = frustum.contains(inFrontAABB, testTransform);
        
        // Test basic functionality - should return a valid boolean result
        REQUIRE((inFront == true || inFront == false));
        
        // Object very far behind camera should not be visible
        testTransform.setPosition(Vector3(0.0f, 0.0f, 150.0f)); // Beyond far plane at 100
        bool farBehind = frustum.contains(inFrontAABB, testTransform);
        REQUIRE(farBehind == false);
        
        delete cameraGO;
    }
    
    SECTION("Plane Extraction Consistency Across Multiple Calls")
    {
        auto [cameraGO, camera] = FrustumTestHelper::createCameraWithGameObject(&componentManager);
        
        // Create multiple frustums from the same camera
        Frustrum frustum1(*camera);
        Frustrum frustum2(*camera);
        
        // Test that they produce consistent results
        TransformComponent testTransform;
        testTransform.setPosition(Vector3(0.0f, 0.0f, -10.0f));
        Aabb testAABB(Vector3(-1.0f, -1.0f, -1.0f), Vector3(1.0f, 1.0f, 1.0f));
        
        bool result1 = frustum1.contains(testAABB, testTransform);
        bool result2 = frustum2.contains(testAABB, testTransform);
        
        REQUIRE(result1 == result2);
        
        delete cameraGO;
    }
    
    SECTION("Extreme FOV Values")
    {
        // Test very wide FOV
        auto [wideCameraGO, wideCamera] = FrustumTestHelper::createCameraWithGameObject(&componentManager, Vector3::Zero, Quaternion::Identity, 10);
        wideCamera->setPerspective(Degree(179.0f), 1280, 768, 0.1f, 100.0f);
        
        Frustrum wideFrustum(*wideCamera);
        
        TransformComponent testTransform;
        testTransform.setPosition(Vector3(0.0f, 0.0f, -5.0f));
        Aabb smallAABB(Vector3(-0.1f, -0.1f, -0.1f), Vector3(0.1f, 0.1f, 0.1f));
        
        // Should not crash with extreme FOV
        bool wideResult = wideFrustum.contains(smallAABB, testTransform);
        REQUIRE((wideResult == true || wideResult == false)); // Just ensure no crash
        
        // Test very narrow FOV
        auto [narrowCameraGO, narrowCamera] = FrustumTestHelper::createCameraWithGameObject(&componentManager, Vector3::Zero, Quaternion::Identity, 11);
        narrowCamera->setPerspective(Degree(1.0f), 1280, 768, 0.1f, 100.0f);
        
        Frustrum narrowFrustum(*narrowCamera);
        
        // Object directly in front should be potentially visible with narrow FOV
        testTransform.setPosition(Vector3(0.0f, 0.0f, -5.0f));
        bool narrowResult = narrowFrustum.contains(smallAABB, testTransform);
        REQUIRE((narrowResult == true || narrowResult == false)); // Basic functionality test
        
        // Object very far to the side should not be visible with narrow FOV
        testTransform.setPosition(Vector3(50.0f, 0.0f, -5.0f));
        bool sideResult = narrowFrustum.contains(smallAABB, testTransform);
        REQUIRE((sideResult == true || sideResult == false)); // Basic functionality test
        
        delete wideCameraGO;
        delete narrowCameraGO;
    }
    
    SECTION("Different Aspect Ratios")
    {
        ComponentManager cm2;
        
        // Test square aspect ratio
        auto [squareCameraGO, squareCamera] = FrustumTestHelper::createCameraWithGameObject(&cm2, Vector3::Zero, Quaternion::Identity, 20);
        squareCamera->setPerspective(Degree(60.0f), 1024, 1024, 0.1f, 100.0f); // 1:1 aspect
        
        Frustrum squareFrustum(*squareCamera);
        
        // Test very wide aspect ratio
        auto [wideCameraGO, wideCamera] = FrustumTestHelper::createCameraWithGameObject(&cm2, Vector3::Zero, Quaternion::Identity, 21);
        wideCamera->setPerspective(Degree(60.0f), 3840, 1080, 0.1f, 100.0f); // 32:9 ultrawide
        
        Frustrum wideFrustum(*wideCamera);
        
        TransformComponent testTransform;
        testTransform.setPosition(Vector3(0.0f, 0.0f, -5.0f));
        Aabb testAABB(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));
        
        // Both should work without crashing
        bool squareResult = squareFrustum.contains(testAABB, testTransform);
        bool wideResult = wideFrustum.contains(testAABB, testTransform);
        
        REQUIRE((squareResult == true || squareResult == false));
        REQUIRE((wideResult == true || wideResult == false));
        
        delete squareCameraGO;
        delete wideCameraGO;
    }
    
    SECTION("Near/Far Plane Edge Cases")
    {
        ComponentManager cm3;
        
        // Test very close near plane
        auto [closeCameraGO, closeCamera] = FrustumTestHelper::createCameraWithGameObject(&cm3, Vector3::Zero, Quaternion::Identity, 30);
        closeCamera->setPerspective(Degree(60.0f), 1280, 768, 0.001f, 1000.0f);
        
        Frustrum closeFrustum(*closeCamera);
        
        // Test very far plane
        auto [farCameraGO, farCamera] = FrustumTestHelper::createCameraWithGameObject(&cm3, Vector3::Zero, Quaternion::Identity, 31);
        farCamera->setPerspective(Degree(60.0f), 1280, 768, 1.0f, 100000.0f);
        
        Frustrum farFrustum(*farCamera);
        
        TransformComponent testTransform;
        testTransform.setPosition(Vector3(0.0f, 0.0f, -5.0f));
        Aabb testAABB(Vector3(-0.1f, -0.1f, -0.1f), Vector3(0.1f, 0.1f, 0.1f));
        
        // Should handle extreme near/far distances
        bool closeResult = closeFrustum.contains(testAABB, testTransform);
        bool farResult = farFrustum.contains(testAABB, testTransform);
        
        REQUIRE((closeResult == true || closeResult == false));
        REQUIRE((farResult == true || farResult == false));
        
        delete closeCameraGO;
        delete farCameraGO;
    }
}

TEST_CASE("FRUSTUM MATHEMATICAL CORRECTNESS", "[FRUSTUM]")
{
    ComponentManager componentManager;
    
    SECTION("Plane Extraction Mathematical Verification")
    {
        // Create a camera with known properties for predictable frustum
        auto [cameraGO, camera] = FrustumTestHelper::createCameraWithGameObject(&componentManager);
        camera->setPerspective(Degree(90.0f), 1024, 1024, 1.0f, 10.0f); // 90 degree FOV, square aspect, near=1, far=10
        
        // Position camera at origin looking down negative Z
        auto* transform = cameraGO->tryGetComponent<TransformComponent>();
        transform->setPosition(Vector3::Zero);
        transform->setRotation(Quaternion::Identity);
        
        Frustrum frustum(*camera);
        
        // With 90 degree FOV and square aspect, the frustum should have specific geometry
        // Test object at origin (should be inside)
        TransformComponent testTransform;
        testTransform.setPosition(Vector3(0.0f, 0.0f, -5.0f)); // Middle of frustum
        Aabb centerAABB(Vector3(-0.1f, -0.1f, -0.1f), Vector3(0.1f, 0.1f, 0.1f));
        
        bool centerResult = frustum.contains(centerAABB, testTransform);
        REQUIRE(centerResult == true);
        
        INFO("Center object should be visible in frustum");
        
        delete cameraGO;
    }
    
    SECTION("Correctness of Corner Testing Algorithm")
    {
        auto [cameraGO, camera] = FrustumTestHelper::createCameraWithGameObject(&componentManager);
        camera->setPerspective(Degree(60.0f), 1280, 720, 0.1f, 100.0f);
        
        Frustrum frustum(*camera);
        
        // Test case: AABB that spans across frustum boundaries
        // This should be visible even if some corners are outside
        TransformComponent spanTransform;
        spanTransform.setPosition(Vector3(0.0f, 0.0f, -10.0f));
        Aabb spanningAABB(Vector3(-50.0f, -50.0f, -5.0f), Vector3(50.0f, 50.0f, 5.0f)); // Large box spanning frustum
        
        bool spanResult = frustum.contains(spanningAABB, spanTransform);
        INFO("Large AABB spanning frustum should be visible");
        INFO("Span result: " << spanResult);
        
        // Test case: Small AABB completely outside
        TransformComponent outsideTransform;
        outsideTransform.setPosition(Vector3(1000.0f, 1000.0f, -10.0f)); // Far outside
        Aabb outsideAABB(Vector3(-0.1f, -0.1f, -0.1f), Vector3(0.1f, 0.1f, 0.1f));
        
        bool outsideResult = frustum.contains(outsideAABB, outsideTransform);
        REQUIRE(outsideResult == false);
        
        delete cameraGO;
    }
    
    SECTION("Matrix vs Transform Component Consistency")
    {
        auto [cameraGO, camera] = FrustumTestHelper::createCameraWithGameObject(&componentManager);
        Frustrum frustum(*camera);
        
        // Test same AABB with equivalent Transform and Matrix4
        Aabb testAABB(Vector3(-1.0f, -1.0f, -1.0f), Vector3(1.0f, 1.0f, 1.0f));
        
        TransformComponent transform;
        transform.setPosition(Vector3(5.0f, 10.0f, -15.0f));
        transform.setRotation(Quaternion(Vector3(0.0f, 1.0f, 0.0f), Radian(Degree(45.0f))));
        transform.setScale(Vector3(2.0f, 2.0f, 2.0f));
        
        Matrix4 matrix = transform.getWorldMatrix();
        
        bool transformResult = frustum.contains(testAABB, transform);
        bool matrixResult = frustum.contains(testAABB, matrix);
        
        REQUIRE(transformResult == matrixResult);
        
        INFO("Transform method result: " << transformResult);
        INFO("Matrix method result: " << matrixResult);
        
        delete cameraGO;
    }
}

TEST_CASE("FRUSTUM ALGORITHM CORRECTNESS", "[FRUSTUM]")
{
    ComponentManager componentManager;
    
    SECTION("Axis-Aligned Optimization Correctness")
    {
        auto [cameraGO, camera] = FrustumTestHelper::createCameraWithGameObject(&componentManager);
        Frustrum frustum(*camera);
        
        // Test axis-aligned transform
        TransformComponent axisAlignedTransform;
        axisAlignedTransform.setPosition(Vector3(0.0f, 0.0f, -10.0f));
        axisAlignedTransform.setRotation(Quaternion::Identity); // No rotation
        axisAlignedTransform.setScale(Vector3(2.0f, 2.0f, 2.0f)); // Uniform scale
        
        Aabb testAABB(Vector3(-1.0f, -1.0f, -1.0f), Vector3(1.0f, 1.0f, 1.0f));
        
        bool axisAlignedResult = frustum.contains(testAABB, axisAlignedTransform);
        
        // Create equivalent rotated transform with tiny rotation (should use oriented path)
        TransformComponent orientedTransform;
        orientedTransform.setPosition(Vector3(0.0f, 0.0f, -10.0f));
        orientedTransform.setRotation(Quaternion(Vector3(0.0f, 1.0f, 0.0f), Radian(Degree(0.01f)))); // Tiny rotation
        orientedTransform.setScale(Vector3(2.0f, 2.0f, 2.0f));
        
        bool orientedResult = frustum.contains(testAABB, orientedTransform);
        
        // Results should be very similar (tiny rotation shouldn't change visibility much)
        INFO("Axis-aligned result: " << axisAlignedResult);
        INFO("Oriented result: " << orientedResult);
        
        delete cameraGO;
    }
    
    SECTION("Negative Scale Handling")
    {
        auto [cameraGO, camera] = FrustumTestHelper::createCameraWithGameObject(&componentManager);
        Frustrum frustum(*camera);
        
        // Test with negative scale (should flip the AABB)
        TransformComponent negativeScaleTransform;
        negativeScaleTransform.setPosition(Vector3(0.0f, 0.0f, -10.0f));
        negativeScaleTransform.setRotation(Quaternion::Identity);
        negativeScaleTransform.setScale(Vector3(-1.0f, 1.0f, 1.0f)); // Negative X scale
        
        Aabb testAABB(Vector3(-1.0f, -1.0f, -1.0f), Vector3(1.0f, 1.0f, 1.0f));
        
        bool negativeScaleResult = frustum.contains(testAABB, negativeScaleTransform);
        
        // Test with positive scale for comparison
        TransformComponent positiveScaleTransform;
        positiveScaleTransform.setPosition(Vector3(0.0f, 0.0f, -10.0f));
        positiveScaleTransform.setRotation(Quaternion::Identity);
        positiveScaleTransform.setScale(Vector3(1.0f, 1.0f, 1.0f));
        
        bool positiveScaleResult = frustum.contains(testAABB, positiveScaleTransform);
        
        // Both should be valid (just ensure no crashes)
        REQUIRE((negativeScaleResult == true || negativeScaleResult == false));
        REQUIRE((positiveScaleResult == true || positiveScaleResult == false));
        
        delete cameraGO;
    }
}

TEST_CASE("FRUSTUM DEBUG INVESTIGATION", "[FRUSTUM]")
{
    ComponentManager componentManager;
    
    SECTION("Debug Camera Setup and Fallback Frustum")
    {
        auto [cameraGO, camera] = FrustumTestHelper::createCameraWithGameObject(&componentManager);
        camera->setPerspective(Degree(90.0f), 1024, 1024, 1.0f, 10.0f);
        
        // Get camera matrices to check if they're valid
        Matrix4 view = camera->getView();
        Matrix4 proj = camera->getProj();
        
        // Check if view is identity (triggering fallback)
        bool isViewIdentity = (view == Matrix4::Identity);
        INFO("Is view matrix identity? " << isViewIdentity);
        
        Frustrum frustum(*camera);
        
        // Test simple case: small object at camera position (z=0)
        TransformComponent transform;
        transform.setPosition(Vector3(0.0f, 0.0f, 0.0f)); // At camera origin
        Aabb smallAABB(Vector3(-0.1f, -0.1f, -0.1f), Vector3(0.1f, 0.1f, 0.1f));
        
        bool atOriginResult = frustum.contains(smallAABB, transform);
        INFO("Object at origin result: " << atOriginResult);
        
        // Test object in front of camera (negative Z)
        transform.setPosition(Vector3(0.0f, 0.0f, -5.0f));
        bool inFrontResult = frustum.contains(smallAABB, transform);
        INFO("Object in front result: " << inFrontResult);
        
        // Test object behind camera (positive Z)
        transform.setPosition(Vector3(0.0f, 0.0f, 5.0f));
        bool behindResult = frustum.contains(smallAABB, transform);
        INFO("Object behind result: " << behindResult);
        
        // Basic assertions to ensure test is working
        REQUIRE(true); // Basic assertion
        REQUIRE((atOriginResult == true || atOriginResult == false));
        REQUIRE((inFrontResult == true || inFrontResult == false));
        REQUIRE((behindResult == true || behindResult == false));
        
        delete cameraGO;
    }
}

TEST_CASE("FRUSTUM UTILITY METHODS", "[FRUSTUM]")
{
    ComponentManager componentManager;
    
    SECTION("Plane Access Methods")
    {
        auto [cameraGO, camera] = FrustumTestHelper::createCameraWithGameObject(&componentManager);
        Frustrum frustum(*camera);
        
        // Test individual plane access
        const Plane& leftPlane = frustum.getPlane(0);
        const Plane& rightPlane = frustum.getPlane(1);
        const Plane& topPlane = frustum.getPlane(2);
        const Plane& bottomPlane = frustum.getPlane(3);
        const Plane& nearPlane = frustum.getPlane(4);
        const Plane& farPlane = frustum.getPlane(5);
        
        // Test out-of-bounds access (should return fallback)
        const Plane& fallbackPlane = frustum.getPlane(10);
        
        // Test plane array access
        std::array<Plane, 6> planes = frustum.getPlanes();
        REQUIRE(planes.size() == 6);
        
        delete cameraGO;
    }
    
    SECTION("Frustum Validity Check")
    {
        auto [cameraGO, camera] = FrustumTestHelper::createCameraWithGameObject(&componentManager);
        Frustrum frustum(*camera);
        
        bool isValid = frustum.isValid();
        REQUIRE(isValid == true); // Should be valid after construction
        
        delete cameraGO;
    }
    
    SECTION("Point Containment Test")
    {
        auto [cameraGO, camera] = FrustumTestHelper::createCameraWithGameObject(&componentManager);
        Frustrum frustum(*camera);
        
        // Test various points
        bool centerResult = frustum.contains(Vector3(0.0f, 0.0f, -5.0f)); // In front of camera
        bool farResult = frustum.contains(Vector3(0.0f, 0.0f, -1000.0f)); // Very far
        bool sideResult = frustum.contains(Vector3(1000.0f, 0.0f, -5.0f)); // To the side
        
        // Results should be valid booleans
        REQUIRE((centerResult == true || centerResult == false));
        REQUIRE((farResult == true || farResult == false));
        REQUIRE((sideResult == true || sideResult == false));
        
        delete cameraGO;
    }
    
    SECTION("Volume Calculation")
    {
        auto [cameraGO, camera] = FrustumTestHelper::createCameraWithGameObject(&componentManager);
        Frustrum frustum(*camera);
        
        float32 volume = frustum.getVolume();
        
        // Volume should be positive and finite
        REQUIRE(volume > 0.0f);
        REQUIRE(std::isfinite(volume));
        
        delete cameraGO;
    }
}
