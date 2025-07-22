#include "catch.hpp"
#include <vector>
#include <cmath>
#include <chrono>

#include "../Engine/Maths/Vector3.hpp"
#include "../Engine/Maths/Vector2.hpp"
#include "../Engine/Core/Types.hpp"

// Include Assimp types for testing
#include <assimp/scene.h>

// Forward declare the functions we want to test
// These are now declared in ModelLoader.cpp and accessible
extern Vector3 buildVertexData(const aiVector3D *vertices, uint32 vertexCount, std::vector<Vector3> &verticesOut);
extern Vector3 calculateCentroid(const aiMesh *mesh);
extern void offsetVertices(std::vector<Vector3> &vertices, const Vector3 &midPoint);

// Helper function to create test data
std::vector<aiVector3D> createTestVertices(const std::vector<Vector3>& testVerts)
{
    std::vector<aiVector3D> aiVerts;
    for (const auto& v : testVerts) {
        aiVector3D aiVert;
        aiVert.x = v.X;
        aiVert.y = v.Y;
        aiVert.z = v.Z;
        aiVerts.push_back(aiVert);
    }
    return aiVerts;
}

TEST_CASE("ModelLoader - buildVertexData Function Tests", "[ModelLoader][Critical]")
{
    SECTION("Basic Average Calculation - Simple Case")
    {
        // Test with simple, known values
        std::vector<Vector3> inputVectors = {
            Vector3(0.0f, 0.0f, 0.0f),
            Vector3(6.0f, 0.0f, 0.0f),
            Vector3(0.0f, 9.0f, 0.0f),
            Vector3(0.0f, 0.0f, 12.0f)
        };
        
        std::vector<aiVector3D> aiVerts = createTestVertices(inputVectors);
        std::vector<Vector3> outputVertices;
        
        Vector3 result = buildVertexData(aiVerts.data(), aiVerts.size(), outputVertices);
        
        // Expected average: (0+6+0+0)/4 = 1.5, (0+0+9+0)/4 = 2.25, (0+0+0+12)/4 = 3.0
        Vector3 expected(1.5f, 2.25f, 3.0f);
        
        REQUIRE(std::abs(result.X - expected.X) < 0.0001f);
        REQUIRE(std::abs(result.Y - expected.Y) < 0.0001f);
        REQUIRE(std::abs(result.Z - expected.Z) < 0.0001f);
        
        // Verify output vertices are copied correctly
        REQUIRE(outputVertices.size() == inputVectors.size());
        for (size_t i = 0; i < inputVectors.size(); ++i) {
            REQUIRE(outputVertices[i].X == inputVectors[i].X);
            REQUIRE(outputVertices[i].Y == inputVectors[i].Y);
            REQUIRE(outputVertices[i].Z == inputVectors[i].Z);
        }
    }
    
    SECTION("Single Vertex - Edge Case")
    {
        std::vector<Vector3> inputVectors = {
            Vector3(5.0f, -3.0f, 2.5f)
        };
        
        std::vector<aiVector3D> aiVerts = createTestVertices(inputVectors);
        std::vector<Vector3> outputVertices;
        
        Vector3 result = buildVertexData(aiVerts.data(), aiVerts.size(), outputVertices);
        
        // Average of single vertex should be the vertex itself
        REQUIRE(std::abs(result.X - 5.0f) < 0.0001f);
        REQUIRE(std::abs(result.Y - (-3.0f)) < 0.0001f);
        REQUIRE(std::abs(result.Z - 2.5f) < 0.0001f);
        
        REQUIRE(outputVertices.size() == 1);
        REQUIRE(outputVertices[0] == inputVectors[0]);
    }
    
    SECTION("Symmetric Values Around Zero")
    {
        std::vector<Vector3> inputVectors = {
            Vector3(-10.0f, -5.0f, -2.0f),
            Vector3(10.0f, 5.0f, 2.0f)
        };
        
        std::vector<aiVector3D> aiVerts = createTestVertices(inputVectors);
        std::vector<Vector3> outputVertices;
        
        Vector3 result = buildVertexData(aiVerts.data(), aiVerts.size(), outputVertices);
        
        // Average should be zero for symmetric values
        REQUIRE(std::abs(result.X) < 0.0001f);
        REQUIRE(std::abs(result.Y) < 0.0001f);
        REQUIRE(std::abs(result.Z) < 0.0001f);
    }
    
    SECTION("Large Number of Vertices - Stress Test")
    {
        std::vector<Vector3> inputVectors;
        Vector3 expectedSum(0.0f);
        
        // Create 1000 vertices with known pattern
        for (int i = 0; i < 1000; ++i) {
            Vector3 vertex(static_cast<float>(i), static_cast<float>(i * 2), static_cast<float>(i * 3));
            inputVectors.push_back(vertex);
            expectedSum += vertex;
        }
        
        Vector3 expectedAverage = expectedSum / 1000.0f;
        
        std::vector<aiVector3D> aiVerts = createTestVertices(inputVectors);
        std::vector<Vector3> outputVertices;
        
        Vector3 result = buildVertexData(aiVerts.data(), aiVerts.size(), outputVertices);
        
        REQUIRE(std::abs(result.X - expectedAverage.X) < 0.01f); // Allow small floating point error
        REQUIRE(std::abs(result.Y - expectedAverage.Y) < 0.01f);
        REQUIRE(std::abs(result.Z - expectedAverage.Z) < 0.01f);
        
        REQUIRE(outputVertices.size() == 1000);
    }
    
    SECTION("Verify Fix - Compare Against Old Buggy Behavior")
    {
        // This test demonstrates that our fix produces different (correct) results
        // compared to the old buggy implementation
        
        std::vector<Vector3> inputVectors = {
            Vector3(2.0f, 4.0f, 6.0f),
            Vector3(8.0f, 10.0f, 12.0f),
            Vector3(14.0f, 16.0f, 18.0f)
        };
        
        std::vector<aiVector3D> aiVerts = createTestVertices(inputVectors);
        std::vector<Vector3> outputVertices;
        
        Vector3 correctResult = buildVertexData(aiVerts.data(), aiVerts.size(), outputVertices);
        
        // Calculate what the old buggy algorithm would have produced
        Vector3 buggyResult(0);
        for (const auto& v : inputVectors) {
            buggyResult.X = (buggyResult.X + v.X) / 2.0f;
            buggyResult.Y = (buggyResult.Y + v.Y) / 2.0f;
            buggyResult.Z = (buggyResult.Z + v.Z) / 2.0f;
        }
        
        // Correct result should be the arithmetic mean
        Vector3 expectedCorrect = (inputVectors[0] + inputVectors[1] + inputVectors[2]) / 3.0f;
        
        // Verify our fix produces the correct result
        REQUIRE(std::abs(correctResult.X - expectedCorrect.X) < 0.0001f);
        REQUIRE(std::abs(correctResult.Y - expectedCorrect.Y) < 0.0001f);
        REQUIRE(std::abs(correctResult.Z - expectedCorrect.Z) < 0.0001f);
        
        // Verify the old buggy result is different (this confirms our fix changed behavior)
        REQUIRE(std::abs(correctResult.X - buggyResult.X) > 0.1f); // Should be significantly different
        REQUIRE(std::abs(correctResult.Y - buggyResult.Y) > 0.1f);
        REQUIRE(std::abs(correctResult.Z - buggyResult.Z) > 0.1f);
    }
}

TEST_CASE("ModelLoader - offsetVertices Function Tests", "[ModelLoader][Supporting]")
{
    SECTION("Basic Offset Operation")
    {
        std::vector<Vector3> vertices = {
            Vector3(10.0f, 20.0f, 30.0f),
            Vector3(15.0f, 25.0f, 35.0f),
            Vector3(5.0f, 15.0f, 25.0f)
        };
        
        Vector3 offset(5.0f, 10.0f, 15.0f);
        
        offsetVertices(vertices, offset);
        
        // Each vertex should be reduced by the offset
        REQUIRE(vertices[0] == Vector3(5.0f, 10.0f, 15.0f));
        REQUIRE(vertices[1] == Vector3(10.0f, 15.0f, 20.0f));
        REQUIRE(vertices[2] == Vector3(0.0f, 5.0f, 10.0f));
    }
    
    SECTION("Zero Offset - No Change")
    {
        std::vector<Vector3> originalVertices = {
            Vector3(1.0f, 2.0f, 3.0f),
            Vector3(4.0f, 5.0f, 6.0f)
        };
        
        std::vector<Vector3> vertices = originalVertices;
        Vector3 zeroOffset(0.0f, 0.0f, 0.0f);
        
        offsetVertices(vertices, zeroOffset);
        
        // Vertices should remain unchanged
        for (size_t i = 0; i < vertices.size(); ++i) {
            REQUIRE(vertices[i] == originalVertices[i]);
        }
    }
    
    SECTION("Negative Offset")
    {
        std::vector<Vector3> vertices = {
            Vector3(0.0f, 0.0f, 0.0f)
        };
        
        Vector3 negativeOffset(-5.0f, -10.0f, -15.0f);
        
        offsetVertices(vertices, negativeOffset);
        
        // Should move vertex in positive direction (subtracting negative is addition)
        REQUIRE(vertices[0] == Vector3(5.0f, 10.0f, 15.0f));
    }
}

TEST_CASE("ModelLoader - Integration Test for Bounding Box Impact", "[ModelLoader][Integration]")
{
    SECTION("Vertex Average Affects Centroid Calculation")
    {
        // This test verifies that the fix to buildVertexData would affect
        // downstream calculations that depend on proper vertex averaging
        
        std::vector<Vector3> testVertices = {
            Vector3(0.0f, 0.0f, 0.0f),
            Vector3(10.0f, 0.0f, 0.0f),
            Vector3(10.0f, 10.0f, 0.0f),
            Vector3(0.0f, 10.0f, 0.0f)
        };
        
        std::vector<aiVector3D> aiVerts = createTestVertices(testVertices);
        std::vector<Vector3> outputVertices;
        
        Vector3 averageVertex = buildVertexData(aiVerts.data(), aiVerts.size(), outputVertices);
        
        // For a square centered at (5,5,0), the average should be the center
        Vector3 expectedCenter(5.0f, 5.0f, 0.0f);
        
        REQUIRE(std::abs(averageVertex.X - expectedCenter.X) < 0.0001f);
        REQUIRE(std::abs(averageVertex.Y - expectedCenter.Y) < 0.0001f);
        REQUIRE(std::abs(averageVertex.Z - expectedCenter.Z) < 0.0001f);
        
        // Now test offset operation as it would be used in mesh construction
        std::vector<Vector3> offsetTestVertices = outputVertices;
        offsetVertices(offsetTestVertices, averageVertex);
        
        // After offsetting by the center, vertices should be centered around origin
        Vector3 newAverage(0.0f);
        for (const auto& v : offsetTestVertices) {
            newAverage += v;
        }
        newAverage = newAverage / static_cast<float>(offsetTestVertices.size());
        
        // New average should be very close to zero
        REQUIRE(std::abs(newAverage.X) < 0.0001f);
        REQUIRE(std::abs(newAverage.Y) < 0.0001f);
        REQUIRE(std::abs(newAverage.Z) < 0.0001f);
    }
}

TEST_CASE("ModelLoader - Performance and Memory Tests", "[ModelLoader][Performance]")
{
    SECTION("Memory Allocation Pattern")
    {
        // Test that output vector is properly reserved and sized
        std::vector<Vector3> largeInput;
        for (int i = 0; i < 10000; ++i) {
            largeInput.push_back(Vector3(static_cast<float>(i), 0.0f, 0.0f));
        }
        
        std::vector<aiVector3D> aiVerts = createTestVertices(largeInput);
        std::vector<Vector3> outputVertices;
        
        // Measure time to ensure reasonable performance
        auto start = std::chrono::high_resolution_clock::now();
        
        Vector3 result = buildVertexData(aiVerts.data(), aiVerts.size(), outputVertices);
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        // Should complete in reasonable time (less than 10ms for 10k vertices)
        REQUIRE(duration.count() < 10000);
        
        // Output should have correct size
        REQUIRE(outputVertices.size() == 10000);
        
        // Result should be mathematically correct for arithmetic sequence
        // Sum of 0 to 9999 = 9999 * 10000 / 2 = 49995000, average = 4999.5
        // Note: Using larger tolerance due to accumulated floating-point precision errors
        REQUIRE(std::abs(result.X - 4999.5f) < 0.5f);
        REQUIRE(std::abs(result.Y) < 0.0001f);
        REQUIRE(std::abs(result.Z) < 0.0001f);
    }
}
