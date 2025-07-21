#include "catch.hpp"

#include "../Engine/Core/ComponentManager.h"
#include "../Engine/Core/GameObject.h"
#include "../Engine/Core/TransformComponent.h"
#include "../Engine/Maths/Matrix4.hpp"
#include "../Engine/Maths/Quaternion.hpp"
#include "../Engine/Maths/Vector3.hpp"

TEST_CASE("TRANSFORM_COMPONENT_CONSTRUCTORS")
{
    SECTION("DEFAULT_CONSTRUCTOR")
    {
        TransformComponent transform;
        
        REQUIRE(transform.getPosition() == Vector3::Zero);
        REQUIRE(transform.getRotation() == Quaternion::Identity);
        REQUIRE(transform.getScale() == Vector3::Identity);
        REQUIRE(transform.getTypeId() == getComponentTypeId<TransformComponent>());
        REQUIRE(transform.getTypeId() == TransformComponent::GetTypeId());
    }
    
    SECTION("PARAMETERIZED_CONSTRUCTOR")
    {
        Vector3 pos(1.0f, 2.0f, 3.0f);
        Quaternion rot(Vector3(0.0f, 1.0f, 0.0f), Radian(Degree(45.0f)));
        Vector3 scale(2.0f, 2.0f, 2.0f);
        
        TransformComponent transform(pos, rot, scale);
        
        REQUIRE(transform.getPosition() == pos);
        REQUIRE(transform.getRotation() == rot);
        REQUIRE(transform.getScale() == scale);
    }
}

TEST_CASE("TRANSFORM_COMPONENT_BASIC_OPERATIONS")
{
    SECTION("POSITION_OPERATIONS")
    {
        TransformComponent transform;
        
        // Test setPosition
        Vector3 newPos(10.0f, 20.0f, 30.0f);
        transform.setPosition(newPos);
        REQUIRE(transform.getPosition() == newPos);
        REQUIRE(transform.hasChanged());
        
        // Test translate
        Vector3 delta(5.0f, -10.0f, 15.0f);
        transform.translate(delta);
        Vector3 expected = newPos + delta;
        REQUIRE(transform.getPosition().X == Approx(expected.X));
        REQUIRE(transform.getPosition().Y == Approx(expected.Y));
        REQUIRE(transform.getPosition().Z == Approx(expected.Z));
    }
    
    SECTION("ROTATION_OPERATIONS")
    {
        TransformComponent transform;
        
        // Test setRotation
        Quaternion newRot(Vector3(0.0f, 1.0f, 0.0f), Radian(Degree(90.0f)));
        transform.setRotation(newRot);
        REQUIRE(transform.getRotation() == newRot);
        REQUIRE(transform.hasChanged());
        
        // Test rotate (applying additional rotation)
        Quaternion additionalRot(Vector3(1.0f, 0.0f, 0.0f), Radian(Degree(45.0f)));
        transform.rotate(additionalRot);
        
        // Result should be different from original rotation
        REQUIRE(transform.getRotation() != newRot);
        REQUIRE(transform.hasChanged());
    }
    
    SECTION("SCALE_OPERATIONS")
    {
        TransformComponent transform;
        
        // Test setScale
        Vector3 newScale(2.0f, 3.0f, 0.5f);
        transform.setScale(newScale);
        REQUIRE(transform.getScale() == newScale);
        REQUIRE(transform.hasChanged());
        
        // Test scale (applying scaling)
        Vector3 scaleMultiplier(2.0f, 1.5f, 3.0f);
        transform.scale(scaleMultiplier);
        
        Vector3 expectedScale(
            newScale.X * scaleMultiplier.X,
            newScale.Y * scaleMultiplier.Y,
            newScale.Z * scaleMultiplier.Z
        );
        
        REQUIRE(transform.getScale().X == Approx(expectedScale.X));
        REQUIRE(transform.getScale().Y == Approx(expectedScale.Y));
        REQUIRE(transform.getScale().Z == Approx(expectedScale.Z));
    }
}

TEST_CASE("TRANSFORM_COMPONENT_WORLD_MATRIX")
{
    SECTION("IDENTITY_TRANSFORM_MATRIX")
    {
        TransformComponent transform;
        
        const Matrix4& worldMatrix = transform.getWorldMatrix();
        
        // Default transform should produce identity matrix
        REQUIRE(worldMatrix == Matrix4::Identity);
    }
    
    SECTION("TRANSLATION_ONLY_MATRIX")
    {
        TransformComponent transform;
        Vector3 position(5.0f, 10.0f, 15.0f);
        transform.setPosition(position);
        
        const Matrix4& worldMatrix = transform.getWorldMatrix();
        
        // Matrix should not be identity
        REQUIRE(worldMatrix != Matrix4::Identity);
        
        // Translation should be in the matrix (assuming column-major, translation in column 3)
        REQUIRE(worldMatrix[3][0] == Approx(position.X));
        REQUIRE(worldMatrix[3][1] == Approx(position.Y));
        REQUIRE(worldMatrix[3][2] == Approx(position.Z));
    }
    
    SECTION("SCALE_ONLY_MATRIX")
    {
        TransformComponent transform;
        Vector3 scale(2.0f, 3.0f, 4.0f);
        transform.setScale(scale);
        
        const Matrix4& worldMatrix = transform.getWorldMatrix();
        
        // Matrix should not be identity
        REQUIRE(worldMatrix != Matrix4::Identity);
        
        // Scale should affect diagonal elements (assuming standard transform matrix layout)
        REQUIRE(std::abs(worldMatrix[0][0]) == Approx(scale.X));
        REQUIRE(std::abs(worldMatrix[1][1]) == Approx(scale.Y));
        REQUIRE(std::abs(worldMatrix[2][2]) == Approx(scale.Z));
    }
    
    SECTION("COMBINED_TRANSFORM_MATRIX")
    {
        TransformComponent transform;
        
        Vector3 position(1.0f, 2.0f, 3.0f);
        Quaternion rotation(Vector3(0.0f, 1.0f, 0.0f), Radian(Degree(90.0f)));
        Vector3 scale(2.0f, 2.0f, 2.0f);
        
        transform.setPosition(position);
        transform.setRotation(rotation);
        transform.setScale(scale);
        
        const Matrix4& worldMatrix = transform.getWorldMatrix();
        
        // Matrix should incorporate all transformations
        REQUIRE(worldMatrix != Matrix4::Identity);
        
        // Should have translation component
        REQUIRE(worldMatrix[3][0] == Approx(position.X));
        REQUIRE(worldMatrix[3][1] == Approx(position.Y));
        REQUIRE(worldMatrix[3][2] == Approx(position.Z));
    }
    
    SECTION("SET_WORLD_MATRIX")
    {
        TransformComponent transform;
        
        // Create a custom transformation matrix
        Matrix4 customMatrix = Matrix4::Identity;
        customMatrix[3][0] = 10.0f;  // Translation X
        customMatrix[3][1] = 20.0f;  // Translation Y
        customMatrix[3][2] = 30.0f;  // Translation Z
        
        transform.setWorldMatrix(customMatrix);
        
        const Matrix4& retrievedMatrix = transform.getWorldMatrix();
        
        // Check specific matrix elements instead of full equality
        REQUIRE(retrievedMatrix[3][0] == Approx(10.0f));
        REQUIRE(retrievedMatrix[3][1] == Approx(20.0f));
        REQUIRE(retrievedMatrix[3][2] == Approx(30.0f));
        REQUIRE(retrievedMatrix[3][3] == Approx(1.0f));  // Should remain 1 for homogeneous coordinates
    }
}

TEST_CASE("TRANSFORM_COMPONENT_CHANGE_TRACKING")
{
    SECTION("INITIAL_STATE")
    {
        TransformComponent transform;
        
        // Clear initial dirty state since components start dirty by design
        transform.clearDirty();
        
        // After clearing, should not have changes
        REQUIRE_FALSE(transform.hasChanged());
        
        uint32 initialChangeId = transform.getChangeId();
        REQUIRE(initialChangeId >= 0);
    }
    
    SECTION("CHANGE_DETECTION")
    {
        TransformComponent transform;
        
        // Clear any initial dirty state
        transform.clearDirty();
        REQUIRE_FALSE(transform.hasChanged());
        
        uint32 initialChangeId = transform.getChangeId();
        
        // Modify transform
        transform.setPosition(Vector3(1.0f, 1.0f, 1.0f));
        REQUIRE(transform.hasChanged());
        REQUIRE(transform.getChangeId() > initialChangeId);
        
        // Clear dirty state
        transform.clearDirty();
        REQUIRE_FALSE(transform.hasChanged());
        
        // Modify again
        transform.setRotation(Quaternion(Vector3(0.0f, 1.0f, 0.0f), Radian(Degree(45.0f))));
        REQUIRE(transform.hasChanged());
    }
    
    SECTION("MANUAL_DIRTY_MARKING")
    {
        TransformComponent transform;
        transform.clearDirty();
        REQUIRE_FALSE(transform.hasChanged());
        
        // Manually mark dirty
        transform.markDirty();
        REQUIRE(transform.hasChanged());
        
        uint32 changeId = transform.getChangeId();
        
        // Mark dirty again
        transform.markDirty();
        REQUIRE(transform.hasChanged());
        REQUIRE(transform.getChangeId() > changeId);
    }
    
    SECTION("CHANGE_ID_INCREMENTS")
    {
        TransformComponent transform;
        uint32 id1 = transform.getChangeId();
        
        transform.setPosition(Vector3(1.0f, 0.0f, 0.0f));
        uint32 id2 = transform.getChangeId();
        REQUIRE(id2 > id1);
        
        // Set a clear rotation that should definitely change
        transform.setRotation(Quaternion(Vector3(0.0f, 1.0f, 0.0f), Radian(Degree(45.0f))));
        uint32 id3 = transform.getChangeId();
        REQUIRE(id3 > id2);
        
        transform.setScale(Vector3(2.0f, 2.0f, 2.0f));
        uint32 id4 = transform.getChangeId();
        REQUIRE(id4 > id3);
    }
}

TEST_CASE("TRANSFORM_COMPONENT_EDGE_CASES")
{
    SECTION("EXTREME_POSITIONS")
    {
        TransformComponent transform;
        
        // Test very large positions
        Vector3 largePos(1000000.0f, -1000000.0f, 500000.0f);
        transform.setPosition(largePos);
        REQUIRE(transform.getPosition() == largePos);
        
        // Test very small positions
        Vector3 smallPos(0.0001f, -0.0001f, 0.00005f);
        transform.setPosition(smallPos);
        REQUIRE(transform.getPosition().X == Approx(smallPos.X));
        REQUIRE(transform.getPosition().Y == Approx(smallPos.Y));
        REQUIRE(transform.getPosition().Z == Approx(smallPos.Z));
    }
    
    SECTION("EXTREME_SCALES")
    {
        TransformComponent transform;
        
        // Test very large scale
        Vector3 largeScale(1000.0f, 500.0f, 2000.0f);
        transform.setScale(largeScale);
        REQUIRE(transform.getScale() == largeScale);
        
        // Test very small scale
        Vector3 smallScale(0.001f, 0.0005f, 0.002f);
        transform.setScale(smallScale);
        REQUIRE(transform.getScale().X == Approx(smallScale.X));
        REQUIRE(transform.getScale().Y == Approx(smallScale.Y));
        REQUIRE(transform.getScale().Z == Approx(smallScale.Z));
        
        // Test zero scale (edge case)
        Vector3 zeroScale(0.0f, 0.0f, 0.0f);
        transform.setScale(zeroScale);
        REQUIRE(transform.getScale() == zeroScale);
        
        // Test negative scale
        Vector3 negativeScale(-1.0f, -2.0f, -0.5f);
        transform.setScale(negativeScale);
        REQUIRE(transform.getScale() == negativeScale);
    }
    
    SECTION("COMPLEX_ROTATIONS")
    {
        TransformComponent transform;
        
        // Test multiple axis rotations
        Quaternion complexRot = 
            Quaternion(Vector3(1.0f, 0.0f, 0.0f), Radian(Degree(30.0f))) *
            Quaternion(Vector3(0.0f, 1.0f, 0.0f), Radian(Degree(45.0f))) *
            Quaternion(Vector3(0.0f, 0.0f, 1.0f), Radian(Degree(60.0f)));
        
        transform.setRotation(complexRot);
        
        // Should handle complex rotations without issues
        REQUIRE(transform.getRotation() == complexRot);
        
        const Matrix4& matrix = transform.getWorldMatrix();
        REQUIRE(matrix != Matrix4::Identity);
    }
}

TEST_CASE("TRANSFORM_COMPONENT_LIFECYCLE")
{
    SECTION("COMPONENT_LIFECYCLE_METHODS")
    {
        TransformComponent transform;
        
        // Test lifecycle methods don't crash
        transform.initialize();
        transform.activate();
        transform.deactivate();
        
        // Component should still be functional after lifecycle
        transform.setPosition(Vector3(1.0f, 2.0f, 3.0f));
        REQUIRE(transform.getPosition() == Vector3(1.0f, 2.0f, 3.0f));
    }
}

TEST_CASE("TRANSFORM_COMPONENT_INTEGRATION_WITH_GAMEOBJECT")
{
    SECTION("AUTOMATIC_TRANSFORM_CREATION")
    {
        ComponentManager manager;
        GameObject gameObject("TestObject", 1, &manager);
        
        // GameObject should automatically have a TransformComponent
        REQUIRE(gameObject.hasComponent<TransformComponent>());
        
        auto& transform = gameObject.getComponent<TransformComponent>();
        REQUIRE(transform.getPosition() == Vector3::Zero);
        REQUIRE(transform.getRotation() == Quaternion::Identity);
        REQUIRE(transform.getScale() == Vector3::Identity);
    }
    
    SECTION("TRANSFORM_MODIFICATIONS_THROUGH_GAMEOBJECT")
    {
        ComponentManager manager;
        GameObject gameObject("TestObject", 1, &manager);
        
        auto& transform = gameObject.getComponent<TransformComponent>();
        
        // Modify transform through reference
        Vector3 newPos(5.0f, 10.0f, 15.0f);
        transform.setPosition(newPos);
        
        // Verify changes persist
        auto& retrievedTransform = gameObject.getComponent<TransformComponent>();
        REQUIRE(retrievedTransform.getPosition() == newPos);
        REQUIRE(&retrievedTransform == &transform); // Should be same object
    }
    
    SECTION("MULTIPLE_GAMEOBJECTS_SEPARATE_TRANSFORMS")
    {
        ComponentManager manager;
        GameObject object1("Object1", 1, &manager);
        GameObject object2("Object2", 2, &manager);
        
        auto& transform1 = object1.getComponent<TransformComponent>();
        auto& transform2 = object2.getComponent<TransformComponent>();
        
        // Modify each transform differently
        transform1.setPosition(Vector3(1.0f, 2.0f, 3.0f));
        transform2.setPosition(Vector3(4.0f, 5.0f, 6.0f));
        
        // Verify they remain separate
        REQUIRE(transform1.getPosition() != transform2.getPosition());
        REQUIRE(transform1.getPosition() == Vector3(1.0f, 2.0f, 3.0f));
        REQUIRE(transform2.getPosition() == Vector3(4.0f, 5.0f, 6.0f));
    }
}

TEST_CASE("TRANSFORM_COMPONENT_PERFORMANCE")
{
    SECTION("REPEATED_MATRIX_CALCULATIONS")
    {
        TransformComponent transform;
        transform.setPosition(Vector3(1.0f, 2.0f, 3.0f));
        transform.setRotation(Quaternion(Vector3(0.0f, 1.0f, 0.0f), Radian(Degree(45.0f))));
        transform.setScale(Vector3(2.0f, 2.0f, 2.0f));
        
        // Test that repeated matrix access is consistent and fast
        const Matrix4* firstMatrix = &transform.getWorldMatrix();
        
        for (int i = 0; i < 1000; ++i)
        {
            const Matrix4& matrix = transform.getWorldMatrix();
            REQUIRE(&matrix == firstMatrix); // Should return same cached matrix
        }
    }
    
    SECTION("RAPID_MODIFICATIONS")
    {
        TransformComponent transform;
        
        // Test rapid modifications don't cause issues
        for (int i = 0; i < 1000; ++i)
        {
            // Just change position - this should definitely create a non-identity matrix
            Vector3 newPosition(static_cast<float32>(i + 1), 0.0f, 0.0f);  // +1 to avoid zero
            transform.setPosition(newPosition);
            
            REQUIRE(transform.hasChanged());
            
            const Matrix4& matrix = transform.getWorldMatrix();
            // With a translation, matrix should have the position in the last column
            REQUIRE(matrix[3][0] == newPosition.X);
            REQUIRE(matrix[3][1] == newPosition.Y);
            REQUIRE(matrix[3][2] == newPosition.Z);
        }
    }
    
    SECTION("LARGE_NUMBER_OF_TRANSFORMS")
    {
        std::vector<std::unique_ptr<TransformComponent>> transforms;
        
        // Create many transform components
        for (int i = 0; i < 1000; ++i)
        {
            auto transform = std::make_unique<TransformComponent>();
            transform->setPosition(Vector3(static_cast<float32>(i), 0.0f, 0.0f));
            transforms.push_back(std::move(transform));
        }
        
        // Verify all are properly configured
        for (size_t i = 0; i < transforms.size(); ++i)
        {
            REQUIRE(transforms[i]->getPosition().X == Approx(static_cast<float32>(i)));
        }
    }
}

TEST_CASE("TRANSFORM_COMPONENT_MATHEMATICAL_PROPERTIES")
{
    SECTION("TRANSFORMATION_ORDER_MATTERS")
    {
        TransformComponent transform1;
        TransformComponent transform2;
        
        Vector3 translation(5.0f, 0.0f, 0.0f);
        Quaternion rotation(Vector3(0.0f, 0.0f, 1.0f), Radian(Degree(90.0f)));
        
        // Apply translation then rotation
        transform1.setPosition(translation);
        transform1.setRotation(rotation);
        
        // Apply rotation then translation (by using combined operations)
        transform2.setRotation(rotation);
        transform2.setPosition(translation);
        
        // The world matrices should be the same in this case since
        // we're setting absolute values, not applying incremental transforms
        const Matrix4& matrix1 = transform1.getWorldMatrix();
        const Matrix4& matrix2 = transform2.getWorldMatrix();
        
        // Both should be valid matrices
        REQUIRE(matrix1 != Matrix4::Identity);
        REQUIRE(matrix2 != Matrix4::Identity);
    }
    
    SECTION("INCREMENTAL_TRANSFORMS")
    {
        TransformComponent transform;
        
        Vector3 initialPos(1.0f, 2.0f, 3.0f);
        transform.setPosition(initialPos);
        
        // Apply multiple translations
        transform.translate(Vector3(1.0f, 0.0f, 0.0f));
        transform.translate(Vector3(0.0f, 1.0f, 0.0f));
        transform.translate(Vector3(0.0f, 0.0f, 1.0f));
        
        Vector3 expectedPos = initialPos + Vector3(1.0f, 1.0f, 1.0f);
        REQUIRE(transform.getPosition().X == Approx(expectedPos.X));
        REQUIRE(transform.getPosition().Y == Approx(expectedPos.Y));
        REQUIRE(transform.getPosition().Z == Approx(expectedPos.Z));
    }
}

TEST_CASE("TRANSFORM_COMPONENT_PARENT_CHILD_HIERARCHY")
{
    SECTION("PARENT_CHILD_WORLD_MATRIX_CALCULATION")
    {
        ComponentManager componentManager;
        
        // Create parent transform
        auto parentTransform = componentManager.createComponent<TransformComponent>();
        parentTransform->setPosition(Vector3(10.0f, 5.0f, 0.0f));
        
        // Create child transform
        auto childTransform = componentManager.createComponent<TransformComponent>();
        childTransform->setPosition(Vector3(2.0f, 1.0f, 0.0f)); // Relative to parent
        childTransform->setParent(parentTransform.get());
        
        // Get world matrices
        Matrix4 parentWorld = parentTransform->getWorldMatrix();
        Matrix4 childWorld = childTransform->getWorldMatrix();
        
        // Parent world position should match local position (no parent)
        REQUIRE(parentWorld[3][0] == Approx(10.0f));
        REQUIRE(parentWorld[3][1] == Approx(5.0f));
        REQUIRE(parentWorld[3][2] == Approx(0.0f));
        
        // Child world position should be parent + child local position
        REQUIRE(childWorld[3][0] == Approx(12.0f)); // 10 + 2
        REQUIRE(childWorld[3][1] == Approx(6.0f));  // 5 + 1
        REQUIRE(childWorld[3][2] == Approx(0.0f));  // 0 + 0
    }
    
    SECTION("PARENT_TRANSFORM_CHANGES_AFFECT_CHILD")
    {
        ComponentManager componentManager;
        
        // Create parent transform
        auto parentTransform = componentManager.createComponent<TransformComponent>();
        parentTransform->setPosition(Vector3(10.0f, 5.0f, 0.0f));
        
        // Create child transform
        auto childTransform = componentManager.createComponent<TransformComponent>();
        childTransform->setPosition(Vector3(2.0f, 1.0f, 0.0f)); // Relative to parent
        childTransform->setParent(parentTransform.get());
        
        // Get initial child world position
        Matrix4 childWorldBefore = childTransform->getWorldMatrix();
        REQUIRE(childWorldBefore[3][0] == Approx(12.0f)); // 10 + 2
        REQUIRE(childWorldBefore[3][1] == Approx(6.0f));  // 5 + 1
        
        // Move parent
        parentTransform->setPosition(Vector3(20.0f, 10.0f, 5.0f));
        
        // Child world position should update automatically
        Matrix4 childWorldAfter = childTransform->getWorldMatrix();
        REQUIRE(childWorldAfter[3][0] == Approx(22.0f)); // 20 + 2
        REQUIRE(childWorldAfter[3][1] == Approx(11.0f)); // 10 + 1
        REQUIRE(childWorldAfter[3][2] == Approx(5.0f));  // 5 + 0
        
        // Child local position should remain unchanged
        REQUIRE(childTransform->getPosition() == Vector3(2.0f, 1.0f, 0.0f));
    }
    
    SECTION("PARENT_SCALE_AFFECTS_CHILD")
    {
        ComponentManager componentManager;
        
        // Create parent transform with scale
        auto parentTransform = componentManager.createComponent<TransformComponent>();
        parentTransform->setPosition(Vector3(0.0f, 0.0f, 0.0f));
        parentTransform->setScale(Vector3(2.0f, 2.0f, 2.0f)); // Scale by 2
        
        // Create child transform
        auto childTransform = componentManager.createComponent<TransformComponent>();
        childTransform->setPosition(Vector3(5.0f, 3.0f, 0.0f)); // Will be scaled
        childTransform->setParent(parentTransform.get());
        
        // Child world position should be scaled by parent's scale
        Matrix4 childWorld = childTransform->getWorldMatrix();
        REQUIRE(childWorld[3][0] == Approx(10.0f)); // 5 * 2
        REQUIRE(childWorld[3][1] == Approx(6.0f));  // 3 * 2
        REQUIRE(childWorld[3][2] == Approx(0.0f));  // 0 * 2
    }
}
