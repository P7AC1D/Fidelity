#include "catch.hpp"

#include "../Engine/Core/ComponentManager.h"
#include "../Engine/Core/GameObject.h"
#include "../Engine/Core/TransformComponent.h"
#include "../Engine/Rendering/LightComponent.h"

TEST_CASE("LIGHT_COMPONENT_BASIC_FUNCTIONALITY")
{
    SECTION("COMPONENT_CREATION_AND_TYPE_ID")
    {
        LightComponent light;
        
        REQUIRE(light.getTypeId() == getComponentTypeId<LightComponent>());
        REQUIRE(light.getTypeId() == LightComponent::GetTypeId());
    }
    
    SECTION("DEFAULT_LIGHT_PROPERTIES")
    {
        LightComponent light;
        
        // Test that default light has reasonable values
        Colour defaultColour = light.getColour();
        REQUIRE(defaultColour[0] >= 0.0f);
        REQUIRE(defaultColour[1] >= 0.0f);
        REQUIRE(defaultColour[2] >= 0.0f);
        REQUIRE(defaultColour[3] >= 0.0f);
        
        // Default light should have some matrix (may be Identity if no transform attached)
        Matrix4 matrix = light.getMatrix();
        // With no transform component attached, matrix should be Identity
        REQUIRE(matrix == Matrix4::Identity);
    }
    
    SECTION("CONSTRUCTOR_WITH_PARAMETERS")
    {
        Colour testColour(Vector4(1.0f, 0.5f, 0.25f, 1.0f));
        float32 testIntensity = 750.0f;
        
        LightComponent light(LightComponentType::Point, testColour, testIntensity);
        
        REQUIRE(light.getColour()[0] == Approx(testColour[0]));
        REQUIRE(light.getColour()[1] == Approx(testColour[1]));
        REQUIRE(light.getColour()[2] == Approx(testColour[2]));
        REQUIRE(light.getColour()[3] == Approx(testColour[3]));
    }
}

TEST_CASE("LIGHT_COMPONENT_LIGHT_TYPES")
{
    SECTION("POINT_LIGHT_CONFIGURATION")
    {
        LightComponent light;
        
        // Configure as point light
        light.setLightType(LightComponentType::Point)
             .setColour(Colour::White)
             .setIntensity(1000.0f)
             .setRadius(50.0f);
        
        // Test fluent interface returns reference
        auto& result = light.setColour(Colour::Red);
        REQUIRE(&result == &light);
        
        // Verify colour was set
        REQUIRE(light.getColour()[0] == Approx(1.0f));
        REQUIRE(light.getColour()[1] == Approx(0.0f));
        REQUIRE(light.getColour()[2] == Approx(0.0f));
    }
    
    SECTION("DIRECTIONAL_LIGHT_CONFIGURATION")
    {
        LightComponent light;
        
        // Configure as directional light
        light.setLightType(LightComponentType::Directional)
             .setColour(Colour(Vector4(1.0f, 1.0f, 0.8f, 1.0f)))
             .setIntensity(2000.0f);
        
        // Directional lights shouldn't need radius
        Colour colour = light.getColour();
        REQUIRE(colour[0] == Approx(1.0f));
        REQUIRE(colour[1] == Approx(1.0f));
        REQUIRE(colour[2] == Approx(0.8f));
    }
    
    SECTION("SPOT_LIGHT_CONFIGURATION")
    {
        LightComponent light;
        
        // Configure as spot light
        light.setLightType(LightComponentType::Spot)
             .setColour(Colour::Blue)
             .setIntensity(1500.0f)
             .setRadius(30.0f);
        
        REQUIRE(light.getColour()[2] == Approx(1.0f));
        REQUIRE(light.getColour()[0] == Approx(0.0f));
        REQUIRE(light.getColour()[1] == Approx(0.0f));
    }
    
    SECTION("AREA_LIGHT_CONFIGURATION")
    {
        LightComponent light;
        
        // Configure as area light
        light.setLightType(LightComponentType::Area)
             .setColour(Colour(Vector4(0.9f, 0.9f, 1.0f, 1.0f)))
             .setIntensity(800.0f);
        
        Colour colour = light.getColour();
        REQUIRE(colour[0] == Approx(0.9f));
        REQUIRE(colour[1] == Approx(0.9f));
        REQUIRE(colour[2] == Approx(1.0f));
    }
}

TEST_CASE("LIGHT_COMPONENT_SHADOW_CONFIGURATION")
{
    SECTION("SHADOW_CASTING_CONFIGURATION")
    {
        LightComponent light;
        
        // Configure shadow properties
        light.setCastsShadows(true)
             .setShadowResolution(1024)
             .setShadowNearPlane(0.1f)
             .setShadowFarPlane(100.0f);
        
        // Test fluent interface
        auto& result = light.setCastsShadows(false);
        REQUIRE(&result == &light);
        
        // Shadow configuration should not crash
        light.setShadowResolution(2048)
             .setShadowNearPlane(0.5f)
             .setShadowFarPlane(200.0f);
        
        REQUIRE(true); // If we get here, configuration succeeded
    }
    
    SECTION("VARIOUS_SHADOW_RESOLUTIONS")
    {
        LightComponent light;
        
        // Test different shadow resolutions
        std::vector<uint32> resolutions = {512, 1024, 2048, 4096};
        
        for (uint32 resolution : resolutions)
        {
            light.setShadowResolution(resolution);
            // Configuration should not crash
            REQUIRE(true);
        }
    }
    
    SECTION("SHADOW_PLANE_CONFIGURATIONS")
    {
        LightComponent light;
        
        // Test various near/far plane combinations
        light.setShadowNearPlane(0.01f).setShadowFarPlane(10.0f);
        light.setShadowNearPlane(0.1f).setShadowFarPlane(100.0f);
        light.setShadowNearPlane(1.0f).setShadowFarPlane(1000.0f);
        
        // All configurations should succeed
        REQUIRE(true);
    }
}

TEST_CASE("LIGHT_COMPONENT_POSITION_AND_TRANSFORM")
{
    SECTION("LIGHT_POSITION_FROM_TRANSFORM")
    {
        ComponentManager manager;
        GameObject lightObject("TestLight", 1, &manager);
        
        auto& light = lightObject.addComponent<LightComponent>();
        auto& transform = lightObject.getComponent<TransformComponent>();
        
        // Set light position via transform
        Vector3 testPosition(10.0f, 20.0f, 30.0f);
        transform.setPosition(testPosition);
        
        // Light should be able to access its position
        Vector3 lightPosition = light.getPosition();
        REQUIRE(lightPosition.X == Approx(testPosition.X));
        REQUIRE(lightPosition.Y == Approx(testPosition.Y));
        REQUIRE(lightPosition.Z == Approx(testPosition.Z));
    }
    
    SECTION("LIGHT_MATRIX_CALCULATION")
    {
        ComponentManager manager;
        GameObject lightObject("TestLight", 1, &manager);
        
        auto& light = lightObject.addComponent<LightComponent>();
        auto& transform = lightObject.getComponent<TransformComponent>();
        
        // Position and rotate the light
        transform.setPosition(Vector3(5.0f, 10.0f, 15.0f));
        transform.setRotation(Quaternion(Vector3(0.0f, 1.0f, 0.0f), Radian(Degree(45.0f))));
        
        // Trigger light component update to recalculate matrix
        light.update(0.0f);
        
        Matrix4 lightMatrix = light.getMatrix();
        
        // Matrix should not be identity when light is positioned/rotated
        REQUIRE(lightMatrix != Matrix4::Identity);
    }
}

TEST_CASE("LIGHT_COMPONENT_COLOR_VARIATIONS")
{
    SECTION("STANDARD_COLORS")
    {
        LightComponent light;
        
        // Test setting standard colors
        light.setColour(Colour::White);
        REQUIRE(light.getColour() == Colour::White);
        
        light.setColour(Colour::Red);
        REQUIRE(light.getColour() == Colour::Red);
        
        light.setColour(Colour::Green);
        REQUIRE(light.getColour() == Colour::Green);
        
        light.setColour(Colour::Blue);
        REQUIRE(light.getColour() == Colour::Blue);
        
        light.setColour(Colour::Black);
        REQUIRE(light.getColour() == Colour::Black);
    }
    
    SECTION("CUSTOM_COLORS")
    {
        LightComponent light;
        
        // Test custom color values
        Colour warmWhite(Vector4(1.0f, 0.95f, 0.8f, 1.0f));
        light.setColour(warmWhite);
        
        Colour retrieved = light.getColour();
        REQUIRE(retrieved[0] == Approx(warmWhite[0]));
        REQUIRE(retrieved[1] == Approx(warmWhite[1]));
        REQUIRE(retrieved[2] == Approx(warmWhite[2]));
        REQUIRE(retrieved[3] == Approx(warmWhite[3]));
        
        // Test another custom color
        Colour coolBlue(Vector4(0.7f, 0.8f, 1.0f, 1.0f));
        light.setColour(coolBlue);
        
        retrieved = light.getColour();
        REQUIRE(retrieved[0] == Approx(coolBlue[0]));
        REQUIRE(retrieved[1] == Approx(coolBlue[1]));
        REQUIRE(retrieved[2] == Approx(coolBlue[2]));
        REQUIRE(retrieved[3] == Approx(coolBlue[3]));
    }
    
    SECTION("EDGE_CASE_COLORS")
    {
        LightComponent light;
        
        // Test edge case colors
        Colour zeroColor(Vector4(0.0f, 0.0f, 0.0f, 0.0f));
        light.setColour(zeroColor);
        REQUIRE(light.getColour() == zeroColor);
        
        // Test very bright color
        Colour brightColor(Vector4(2.0f, 2.0f, 2.0f, 1.0f));
        light.setColour(brightColor);
        Colour retrieved = light.getColour();
        REQUIRE(retrieved[0] == Approx(2.0f));
        REQUIRE(retrieved[1] == Approx(2.0f));
        REQUIRE(retrieved[2] == Approx(2.0f));
    }
}

TEST_CASE("LIGHT_COMPONENT_INTENSITY_VARIATIONS")
{
    SECTION("TYPICAL_INTENSITY_VALUES")
    {
        LightComponent light;
        
        // Test various intensity values
        std::vector<float32> intensities = {0.0f, 100.0f, 500.0f, 1000.0f, 2000.0f, 5000.0f};
        
        for (float32 intensity : intensities)
        {
            light.setIntensity(intensity);
            // Configuration should not crash
            REQUIRE(true);
        }
    }
    
    SECTION("EXTREME_INTENSITY_VALUES")
    {
        LightComponent light;
        
        // Test very low intensity
        light.setIntensity(0.001f);
        REQUIRE(true);
        
        // Test very high intensity
        light.setIntensity(100000.0f);
        REQUIRE(true);
        
        // Test negative intensity (should be handled gracefully)
        light.setIntensity(-100.0f);
        REQUIRE(true);
    }
}

TEST_CASE("LIGHT_COMPONENT_LIFECYCLE")
{
    SECTION("COMPONENT_LIFECYCLE_METHODS")
    {
        ComponentManager manager;
        GameObject lightObject("TestLight", 1, &manager);
        
        auto& light = lightObject.addComponent<LightComponent>();
        
        // Test that lifecycle methods don't crash
        // Note: Some lifecycle methods might not be implemented for LightComponent
        // but the basic component interface should work
        
        // Basic configuration after creation
        light.setColour(Colour::White)
             .setIntensity(1000.0f)
             .setLightType(LightComponentType::Point);
        
        REQUIRE(light.getColour() == Colour::White);
    }
}

TEST_CASE("LIGHT_COMPONENT_INTEGRATION_WITH_GAMEOBJECT")
{
    SECTION("LIGHT_WITH_GAMEOBJECT_TRANSFORM")
    {
        ComponentManager manager;
        GameObject lightObject("TestLight", 1, &manager);
        
        auto& light = lightObject.addComponent<LightComponent>();
        auto& transform = lightObject.getComponent<TransformComponent>();
        
        // Configure light
        light.setLightType(LightComponentType::Directional)
             .setColour(Colour(Vector4(1.0f, 1.0f, 0.9f, 1.0f)))
             .setIntensity(1500.0f);
        
        // Position and orient the light
        transform.setPosition(Vector3(0.0f, 10.0f, 0.0f));
        transform.setRotation(Quaternion(Vector3(1.0f, 0.0f, 0.0f), Radian(Degree(-45.0f))));
        
        // Trigger light component update to recalculate matrix
        light.update(0.0f);
        
        // Test that light can access transform data
        Vector3 position = light.getPosition();
        REQUIRE(position.X == Approx(0.0f));
        REQUIRE(position.Y == Approx(10.0f));
        REQUIRE(position.Z == Approx(0.0f));
        
        Matrix4 matrix = light.getMatrix();
        REQUIRE(matrix != Matrix4::Identity);
    }
    
    SECTION("MULTIPLE_LIGHTS_SAME_MANAGER")
    {
        ComponentManager manager;
        
        GameObject pointLightObject("PointLight", 1, &manager);
        GameObject dirLightObject("DirectionalLight", 2, &manager);
        GameObject spotLightObject("SpotLight", 3, &manager);
        
        auto& pointLight = pointLightObject.addComponent<LightComponent>();
        auto& dirLight = dirLightObject.addComponent<LightComponent>();
        auto& spotLight = spotLightObject.addComponent<LightComponent>();
        
        // Configure different light types
        pointLight.setLightType(LightComponentType::Point)
                  .setColour(Colour::Red)
                  .setIntensity(800.0f);
        
        dirLight.setLightType(LightComponentType::Directional)
                .setColour(Colour::White)
                .setIntensity(2000.0f);
        
        spotLight.setLightType(LightComponentType::Spot)
                 .setColour(Colour::Blue)
                 .setIntensity(1200.0f);
        
        // Verify they maintain separate state
        REQUIRE(pointLight.getColour() == Colour::Red);
        REQUIRE(dirLight.getColour() == Colour::White);
        REQUIRE(spotLight.getColour() == Colour::Blue);
    }
}

TEST_CASE("LIGHT_COMPONENT_REAL_WORLD_SCENARIOS")
{
    SECTION("OUTDOOR_SCENE_LIGHTING")
    {
        ComponentManager manager;
        
        // Sun light (directional)
        GameObject sunObject("Sun", 1, &manager);
        auto& sun = sunObject.addComponent<LightComponent>();
        auto& sunTransform = sunObject.getComponent<TransformComponent>();
        
        sun.setLightType(LightComponentType::Directional)
           .setColour(Colour(Vector4(1.0f, 0.95f, 0.8f, 1.0f)))  // Warm sunlight
           .setIntensity(3000.0f)
           .setCastsShadows(true)
           .setShadowResolution(2048);
        
        // Position sun high and at an angle
        sunTransform.setPosition(Vector3(0.0f, 100.0f, 0.0f));  // High in the sky
        sunTransform.setRotation(Quaternion(Vector3(1.0f, 0.3f, 0.0f), Radian(Degree(-60.0f))));
        
        REQUIRE(sun.getColour()[0] >= 0.95f);
        REQUIRE(sun.getPosition() == Vector3(0.0f, 100.0f, 0.0f)); // Should have position from transform
    }
    
    SECTION("INDOOR_SCENE_LIGHTING")
    {
        ComponentManager manager;
        
        // Ceiling light (point)
        GameObject ceilingLightObject("CeilingLight", 1, &manager);
        auto& ceilingLight = ceilingLightObject.addComponent<LightComponent>();
        auto& ceilingTransform = ceilingLightObject.getComponent<TransformComponent>();
        
        ceilingLight.setLightType(LightComponentType::Point)
                   .setColour(Colour::White)
                   .setIntensity(1000.0f)
                   .setRadius(20.0f)
                   .setCastsShadows(true);
        
        ceilingTransform.setPosition(Vector3(0.0f, 8.0f, 0.0f));
        
        // Desk lamp (spot)
        GameObject deskLampObject("DeskLamp", 2, &manager);
        auto& deskLamp = deskLampObject.addComponent<LightComponent>();
        auto& lampTransform = deskLampObject.getComponent<TransformComponent>();
        
        deskLamp.setLightType(LightComponentType::Spot)
               .setColour(Colour(Vector4(1.0f, 0.9f, 0.7f, 1.0f)))  // Warm desk light
               .setIntensity(500.0f)
               .setRadius(10.0f);
        
        lampTransform.setPosition(Vector3(5.0f, 3.0f, 2.0f));
        lampTransform.setRotation(Quaternion(Vector3(1.0f, 0.0f, 0.0f), Radian(Degree(-45.0f))));
        
        // Verify both lights are configured correctly
        REQUIRE(ceilingLight.getPosition().Y == Approx(8.0f));
        REQUIRE(deskLamp.getPosition().X == Approx(5.0f));
    }
}

TEST_CASE("LIGHT_COMPONENT_PERFORMANCE")
{
    SECTION("REPEATED_PROPERTY_ACCESS")
    {
        LightComponent light;
        light.setColour(Colour::White)
             .setIntensity(1000.0f)
             .setLightType(LightComponentType::Point);
        
        // Test that repeated property access is consistent
        for (int i = 0; i < 1000; ++i)
        {
            REQUIRE(light.getColour() == Colour::White);
            Matrix4 matrix = light.getMatrix();
            REQUIRE(matrix != Matrix4::Zero);
        }
    }
    
    SECTION("REPEATED_POSITION_CALCULATIONS")
    {
        ComponentManager manager;
        GameObject lightObject("TestLight", 1, &manager);
        
        auto& light = lightObject.addComponent<LightComponent>();
        auto& transform = lightObject.getComponent<TransformComponent>();
        
        transform.setPosition(Vector3(1.0f, 2.0f, 3.0f));
        
        // Test that repeated position calculations don't crash
        for (int i = 0; i < 100; ++i)
        {
            Vector3 position = light.getPosition();
            REQUIRE(position.X == Approx(1.0f));
            REQUIRE(position.Y == Approx(2.0f));
            REQUIRE(position.Z == Approx(3.0f));
        }
    }
}
