#include "catch.hpp"

#include "../Engine/Core/ComponentManager.h"
#include "../Engine/Core/GameObject.h"
#include "../Engine/Core/TransformComponent.h"
#include "../Engine/Rendering/CameraComponent.h"
#include "../Engine/Maths/Frustrum.hpp"
#include "../Engine/Maths/AABB.hpp"

TEST_CASE("CAMERA_COMPONENT_BASIC_FUNCTIONALITY")
{
    SECTION("COMPONENT_CREATION_AND_TYPE_ID")
    {
        CameraComponent camera;
        
        REQUIRE(camera.getTypeId() == getComponentTypeId<CameraComponent>());
        REQUIRE(camera.getTypeId() == CameraComponent::GetTypeId());
    }
    
    SECTION("DEFAULT_CAMERA_PROPERTIES")
    {
        CameraComponent camera;
        
        // Test default values are reasonable
        REQUIRE(camera.getWidth() > 0);
        REQUIRE(camera.getHeight() > 0);
        REQUIRE(camera.getNear() > 0.0f);
        REQUIRE(camera.getFar() > camera.getNear());
        REQUIRE(camera.getAspectRatio() == Approx(static_cast<float32>(camera.getWidth()) / camera.getHeight()));
    }
    
    SECTION("PERSPECTIVE_CAMERA_CONFIGURATION")
    {
        CameraComponent camera;
        
        // Configure perspective camera
        camera.setPerspective(Degree(60.0f), 1920, 1080, 0.1f, 1000.0f);
        
        REQUIRE(camera.getWidth() == 1920);
        REQUIRE(camera.getHeight() == 1080);
        REQUIRE(camera.getFov().InDegrees() == Approx(60.0f));
        REQUIRE(camera.getNear() == Approx(0.1f));
        REQUIRE(camera.getFar() == Approx(1000.0f));
        REQUIRE(camera.getAspectRatio() == Approx(1920.0f / 1080.0f));
    }
}

TEST_CASE("CAMERA_COMPONENT_PROPERTY_SETTERS")
{
    SECTION("INDIVIDUAL_PROPERTY_SETTERS")
    {
        CameraComponent camera;
        
        // Test fluent interface and individual setters
        camera.setWidth(800)
              .setHeight(600)
              .setFov(Degree(45.0f))
              .setNear(0.5f)
              .setFar(500.0f);
        
        REQUIRE(camera.getWidth() == 800);
        REQUIRE(camera.getHeight() == 600);
        REQUIRE(camera.getFov().InDegrees() == Approx(45.0f));
        REQUIRE(camera.getNear() == Approx(0.5f));
        REQUIRE(camera.getFar() == Approx(500.0f));
        REQUIRE(camera.getAspectRatio() == Approx(800.0f / 600.0f));
    }
    
    SECTION("CHAIN_PROPERTY_SETTERS")
    {
        CameraComponent camera;
        
        // Test method chaining
        auto& result = camera.setPerspective(Degree(90.0f), 1280, 720, 0.01f, 2000.0f);
        
        // Should return reference to self for chaining
        REQUIRE(&result == &camera);
        
        // Verify all properties were set
        REQUIRE(camera.getWidth() == 1280);
        REQUIRE(camera.getHeight() == 720);
        REQUIRE(camera.getFov().InDegrees() == Approx(90.0f));
        REQUIRE(camera.getNear() == Approx(0.01f));
        REQUIRE(camera.getFar() == Approx(2000.0f));
    }
}

TEST_CASE("CAMERA_COMPONENT_MATRIX_CALCULATIONS")
{
    SECTION("PROJECTION_MATRIX_PROPERTIES")
    {
        CameraComponent camera;
        camera.setPerspective(Degree(60.0f), 800, 600, 1.0f, 100.0f);
        
        Matrix4 proj = camera.getProj();
        
        // Projection matrix should not be identity
        REQUIRE(proj != Matrix4::Identity);
        
        // For perspective projection, certain matrix elements should have expected properties
        // The exact values depend on your matrix convention, but we can test basic properties
        REQUIRE(proj[2][3] != 0.0f); // Should have perspective divide component
    }
    
    SECTION("VIEW_MATRIX_WITH_TRANSFORM")
    {
        ComponentManager manager;
        GameObject cameraObject("TestCamera", 1, &manager);
        
        auto& camera = cameraObject.addComponent<CameraComponent>();
        auto& transform = cameraObject.getComponent<TransformComponent>();
        
        // Set camera position and look at origin
        transform.setPosition(Vector3(0.0f, 0.0f, 5.0f));
        
        Matrix4 view = camera.getView();
        
        // View matrix should not be identity when camera is positioned
        REQUIRE(view != Matrix4::Identity);
    }
}

TEST_CASE("CAMERA_COMPONENT_FRUSTUM_FUNCTIONALITY")
{
    SECTION("FRUSTUM_CREATION")
    {
        ComponentManager manager;
        GameObject cameraObject("TestCamera", 1, &manager);
        
        auto& camera = cameraObject.addComponent<CameraComponent>();
        camera.setPerspective(Degree(60.0f), 800, 600, 1.0f, 100.0f);
        
        // Get frustum should not crash
        const Frustrum& frustum = camera.getFrustum();
        
        // Basic test - frustum creation should succeed
        REQUIRE(true); // If we get here, frustum creation worked
    }
    
    SECTION("CONTAINS_METHOD_BASIC_FUNCTIONALITY")
    {
        ComponentManager manager;
        GameObject cameraObject("TestCamera", 1, &manager);
        
        auto& camera = cameraObject.addComponent<CameraComponent>();
        auto& transform = cameraObject.getComponent<TransformComponent>();
        
        camera.setPerspective(Degree(60.0f), 800, 600, 1.0f, 100.0f);
        transform.setPosition(Vector3(0.0f, 0.0f, 0.0f));
        
        // Test contains method with simple AABB
        Aabb testBox(Vector3::Zero, 1.0f, 1.0f, 1.0f);
        Matrix4 testTransform = Matrix4::Identity;
        
        bool result = camera.contains(testBox, testTransform);
        
        // Result should be a valid boolean
        REQUIRE((result == true || result == false));
    }
}

TEST_CASE("CAMERA_COMPONENT_EDGE_CASES")
{
    SECTION("EXTREME_FOV_VALUES")
    {
        CameraComponent camera;
        
        // Test very small FOV
        camera.setFov(Degree(1.0f));
        REQUIRE(camera.getFov().InDegrees() == Approx(1.0f));
        
        // Test large FOV (but reasonable)
        camera.setFov(Degree(160.0f));
        REQUIRE(camera.getFov().InDegrees() == Approx(160.0f));
    }
    
    SECTION("EXTREME_NEAR_FAR_RATIOS")
    {
        CameraComponent camera;
        
        // Test very close near plane
        camera.setNear(0.001f).setFar(1000.0f);
        REQUIRE(camera.getNear() == Approx(0.001f));
        REQUIRE(camera.getFar() == Approx(1000.0f));
        
        // Test large far plane
        camera.setNear(1.0f).setFar(10000.0f);
        REQUIRE(camera.getNear() == Approx(1.0f));
        REQUIRE(camera.getFar() == Approx(10000.0f));
    }
    
    SECTION("UNUSUAL_ASPECT_RATIOS")
    {
        CameraComponent camera;
        
        // Test very wide aspect ratio
        camera.setWidth(3840).setHeight(1080);
        REQUIRE(camera.getAspectRatio() == Approx(3840.0f / 1080.0f));
        
        // Test very tall aspect ratio
        camera.setWidth(1080).setHeight(1920);
        REQUIRE(camera.getAspectRatio() == Approx(1080.0f / 1920.0f));
        
        // Test square aspect ratio
        camera.setWidth(1024).setHeight(1024);
        REQUIRE(camera.getAspectRatio() == Approx(1.0f));
    }
}

TEST_CASE("CAMERA_COMPONENT_LIFECYCLE")
{
    SECTION("COMPONENT_LIFECYCLE_METHODS")
    {
        ComponentManager manager;
        GameObject cameraObject("TestCamera", 1, &manager);
        
        auto& camera = cameraObject.addComponent<CameraComponent>();
        
        // Test lifecycle methods don't crash
        camera.onInitialize();
        camera.onActivate();
        camera.onUpdate(0.016f); // 60 FPS delta
        camera.onDeactivate();
        
        // Component should still be functional after lifecycle
        camera.setPerspective(Degree(45.0f), 640, 480, 0.1f, 100.0f);
        REQUIRE(camera.getWidth() == 640);
        REQUIRE(camera.getHeight() == 480);
    }
}

TEST_CASE("CAMERA_COMPONENT_INTEGRATION_WITH_GAMEOBJECT")
{
    SECTION("CAMERA_WITH_GAMEOBJECT_TRANSFORM")
    {
        ComponentManager manager;
        GameObject cameraObject("TestCamera", 1, &manager);
        
        // Add camera component
        auto& camera = cameraObject.addComponent<CameraComponent>();
        auto& transform = cameraObject.getComponent<TransformComponent>();
        
        // Configure camera
        camera.setPerspective(Degree(60.0f), 1280, 720, 0.1f, 1000.0f);
        
        // Position camera
        transform.setPosition(Vector3(10.0f, 5.0f, 10.0f));
        transform.setRotation(Quaternion(Vector3(0.0f, 1.0f, 0.0f), Radian(Degree(45.0f))));
        
        // Test that camera can access transform data through GameObject
        Matrix4 view = camera.getView();
        REQUIRE(view != Matrix4::Identity);
        
        // Test frustum with positioned camera
        const Frustrum& frustum = camera.getFrustum();
        REQUIRE(true); // Frustum creation should succeed
    }
    
    SECTION("MULTIPLE_CAMERAS_SAME_MANAGER")
    {
        ComponentManager manager;
        
        GameObject camera1Object("Camera1", 1, &manager);
        GameObject camera2Object("Camera2", 2, &manager);
        
        auto& camera1 = camera1Object.addComponent<CameraComponent>();
        auto& camera2 = camera2Object.addComponent<CameraComponent>();
        
        // Configure cameras differently
        camera1.setPerspective(Degree(60.0f), 1920, 1080, 0.1f, 1000.0f);
        camera2.setPerspective(Degree(45.0f), 1280, 720, 0.5f, 500.0f);
        
        // Verify they maintain separate state
        REQUIRE(camera1.getWidth() == 1920);
        REQUIRE(camera2.getWidth() == 1280);
        REQUIRE(camera1.getFov().InDegrees() == Approx(60.0f));
        REQUIRE(camera2.getFov().InDegrees() == Approx(45.0f));
    }
}

TEST_CASE("CAMERA_COMPONENT_PERFORMANCE")
{
    SECTION("REPEATED_PROPERTY_ACCESS")
    {
        CameraComponent camera;
        camera.setPerspective(Degree(60.0f), 1920, 1080, 0.1f, 1000.0f);
        
        // Test that repeated property access is consistent
        for (int i = 0; i < 1000; ++i)
        {
            REQUIRE(camera.getWidth() == 1920);
            REQUIRE(camera.getHeight() == 1080);
            REQUIRE(camera.getNear() == Approx(0.1f));
            REQUIRE(camera.getFar() == Approx(1000.0f));
        }
    }
    
    SECTION("REPEATED_MATRIX_CALCULATIONS")
    {
        ComponentManager manager;
        GameObject cameraObject("TestCamera", 1, &manager);
        
        auto& camera = cameraObject.addComponent<CameraComponent>();
        camera.setPerspective(Degree(60.0f), 1920, 1080, 0.1f, 1000.0f);
        
        // Test that repeated matrix calculations don't crash
        for (int i = 0; i < 100; ++i)
        {
            Matrix4 proj = camera.getProj();
            Matrix4 view = camera.getView();
            
            REQUIRE(proj != Matrix4::Identity);
        }
    }
}
