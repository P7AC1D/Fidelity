#pragma once

// Test-only header for exposing ModelLoader internal functions
// This should only be included in test files

#include "../Engine/Maths/Vector3.hpp"
#include "../Engine/Maths/Vector2.hpp"
#include "../Engine/Core/Types.hpp"

#include <vector>
#include <assimp/scene.h>

// Forward declarations for test access to internal ModelLoader functions
namespace ModelLoaderTestAccess {
    
// Expose internal functions for testing
Vector3 buildVertexData(const aiVector3D *vertices, uint32 vertexCount, std::vector<Vector3> &verticesOut);
Vector3 calculateCentroid(const aiMesh *mesh);
void offsetVertices(std::vector<Vector3> &vertices, const Vector3 &midPoint);
void buildIndexData(const aiFace *faces, uint32 indexCount, std::vector<uint32> &indicesOut);
void buildTexCoordData(const aiVector3D *texCoords, uint32 texCoordCount, std::vector<Vector2> &texCoordsOut);
void buildNormalData(const aiVector3D *normals, uint32 normalCount, std::vector<Vector3> &normalsOut);

} // namespace ModelLoaderTestAccess
