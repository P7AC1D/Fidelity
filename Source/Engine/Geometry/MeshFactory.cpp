#include "MeshFactory.h"

#include <unordered_map>
#include <vector>

#include "../Maths/Vector2.hpp"
#include "../Maths/Vector3.hpp"
#include "../Rendering/Material.hpp"
#include "../Rendering/MaterialFactory.hpp"
#include "../Rendering/StaticMesh.h"
#include "Cube.hpp"
#include "Icosphere.hpp"

std::shared_ptr<StaticMesh> MeshFactory::CreateCube()
{
  Cube cube;
  auto mesh = std::make_shared<StaticMesh>();
  mesh->SetPositionVertexData(cube.GetPositions());
  mesh->SetTextureVertexData(cube.GetTexCoords());
  mesh->SetIndexData(cube.GetIndices());
  mesh->GenerateNormals();
  mesh->GenerateTangents();
  return mesh;
}

std::shared_ptr<StaticMesh> MeshFactory::CreatePlane(uint32 density)
{
	float32 interval = 1.0f / static_cast<float32>(density);

	uint32 vertexWidth = density + 2;
	uint32 vertexCount = vertexWidth * vertexWidth;
	uint32 faceCount = (density + 1) * (density + 1);
	uint32 indexCount = faceCount * 6;

  std::vector<Vector3> positions;
  std::vector<Vector2> texCoords;
	std::vector<uint32> indices;

	positions.reserve(vertexCount);
	texCoords.reserve(vertexCount);
	indices.reserve(indexCount);

  for (uint32 i = 0; i < vertexWidth; i++)
  {
    float32 y = -1.0f + i * interval;
    for (uint32 j = 0; j < vertexWidth; j++)
    {
      float32 x = -1.0f + j * interval;
      positions.push_back(Vector3(x, 0.0f, y));
      texCoords.push_back(Vector2(i, j));
    }
  }

	uint32 offset = 0;
	for (uint32 i = 0; i < indexCount; i = i + 6)
	{
		uint32 cornerIndex = i / 6 + offset;
		if ((cornerIndex + 1) % vertexWidth == 0)
		{
			offset++;
			cornerIndex++;
		}

		indices.push_back(cornerIndex);
		indices.push_back(cornerIndex + vertexWidth);
		indices.push_back(cornerIndex + vertexWidth + 1);

		indices.push_back(cornerIndex);
		indices.push_back(cornerIndex + vertexWidth + 1);
		indices.push_back(cornerIndex + 1);
	}

  auto mesh = std::make_shared<StaticMesh>();
  mesh->SetPositionVertexData(positions);
  mesh->SetTextureVertexData(texCoords);
	mesh->SetIndexData(indices);
	mesh->GenerateNormals();
	mesh->GenerateTangents();
  return mesh;
}

std::shared_ptr<StaticMesh> MeshFactory::CreateIcosphere(uint32 recursionCount)
{
  Icosphere icosphere(recursionCount);
  auto mesh = std::make_shared<StaticMesh>();
  mesh->SetPositionVertexData(icosphere.GetPositions());  
  mesh->SetTextureVertexData(icosphere.GetTexCoords());
  mesh->SetIndexData(icosphere.GetIndices());
  mesh->GenerateNormals();
  mesh->GenerateTangents();
  return mesh;
}
