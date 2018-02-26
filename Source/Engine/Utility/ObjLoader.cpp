#include "ObjLoader.hpp"

#include <algorithm>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "../Maths/Colour.hpp"
#include "../Maths/Vector2.hpp"
#include "../Maths/Vector3.hpp"
#include "../Rendering/Material.h"
#include "../Rendering/Renderable.hpp"
#include "../Rendering/StaticMesh.h"
#include "../Utility/AssetManager.h"
#include "../Utility/StringUtil.h"

using namespace Rendering;

namespace Utility
{
void BuildIndexData(const aiFace* faces, uint32 indexCount, std::vector<uint32>& indicesOut)
{
  indicesOut.reserve(indexCount);
  for (uint32 i = 0; i < indexCount; i++)
  {
    auto face = faces + i;
    if (face->mNumIndices != 3)
    {
      throw std::runtime_error("Non-triangle face read");
    }
    indicesOut.push_back(faces->mIndices[0]);
    indicesOut.push_back(faces->mIndices[1]);
    indicesOut.push_back(faces->mIndices[2]);
  }
}

void BuildTexCoordData(const aiVector3D* texCoords, uint32 texCoordCount, std::vector<Vector2>& texCoordsOut)
{
  texCoordsOut.reserve(texCoordCount);
  for (uint32 i = 0; i < texCoordCount; i++)
  {
    texCoordsOut.emplace_back(texCoords[i].x, texCoords[i].y);
  }
}

void BuildVertexData(const aiVector3D* vertices, uint32 verexCount, std::vector<Vector3>& verticesOut)
{
  verticesOut.reserve(verexCount);
  for (uint32 i = 0; i < verexCount; i++)
  {
    verticesOut.emplace_back(vertices[i].x, vertices[i].y, vertices[i].z);
  }
}

void BuildMaterial(const std::string& filePath, const aiMaterial* aiMaterial, std::shared_ptr<Rendering::Material> material)
{
  aiColor3D ambientColour;
  aiMaterial->Get(AI_MATKEY_COLOR_AMBIENT, ambientColour);
  material->SetAmbientColour(Vector3(ambientColour.r, ambientColour.g, ambientColour.b));

  aiColor3D diffuseColour;
  aiMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColour);
  material->SetDiffuseColour(Vector3(diffuseColour.r, diffuseColour.g, diffuseColour.b));

  aiColor3D specularColour;
  aiMaterial->Get(AI_MATKEY_COLOR_SPECULAR, specularColour);
  material->SetSpecularColour(Vector3(specularColour.r, specularColour.g, specularColour.b));

  float32 specularShininess;
  aiMaterial->Get(AI_MATKEY_SHININESS, specularShininess);
  material->SetSpecularShininess(specularShininess);

  aiString diffuseTexturePath;
  aiMaterial->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), diffuseTexturePath);
  if (diffuseTexturePath.length != 0)
  {
    material->SetTexture("DiffuseMap", AssetManager::GetTexture(filePath + diffuseTexturePath.C_Str()));
  }
}

std::shared_ptr<StaticMesh> BuildMesh(const std::string& filePath, const aiMesh* aiMesh, const aiMaterial* aiMaterial)
{
  if (!aiMesh->HasPositions() || !aiMesh->HasNormals())
  {
    return nullptr;
  }

  std::shared_ptr<StaticMesh> mesh(new StaticMesh(aiMesh->mName.C_Str()));
  auto material = mesh->GetMaterial();
  BuildMaterial(filePath, aiMaterial, material);

  std::vector<Vector3> vertices;
  std::vector<Vector3> normals;
  BuildVertexData(aiMesh->mVertices, aiMesh->mNumVertices, vertices);
  BuildVertexData(aiMesh->mNormals, aiMesh->mNumVertices, normals);

  mesh->SetPositionVertexData(vertices);
  mesh->SetNormalVertexData(normals);

  // Assume that mesh contains a single set of texture coordinate data.
  if (aiMesh->HasTextureCoords(0))
  {
    std::vector<Vector2> texCoords;
    BuildTexCoordData(aiMesh->mTextureCoords[0], aiMesh->mNumUVComponents[0], texCoords);
    mesh->SetTextureVertexData(texCoords);
  }

  if (aiMesh->HasFaces())
  {
    std::vector<uint32> indices;
    BuildIndexData(aiMesh->mFaces, aiMesh->mNumFaces, indices);
  }

  return mesh;
}

std::shared_ptr<Renderable> BuildModel(const std::string& filePath, const aiScene* scene)
{
  if (!scene->HasMeshes() || !scene->HasMaterials())
  {
    return nullptr;
  }

  std::shared_ptr<Renderable> renderable(new Renderable);
  for (uint32 i = 0; i < scene->mNumMeshes; i++)
  {
    auto aiMesh = scene->mMeshes[i];
    auto mesh = BuildMesh(filePath, aiMesh, scene->mMaterials[aiMesh->mMaterialIndex]);
    renderable->PushMesh(mesh);
  }

  return renderable;
}

std::shared_ptr<Renderable> ObjLoader::LoadFromFile(const std::string& filePath, const std::string& fileName)
{
  Assimp::Importer importer;
  auto scene = importer.ReadFile(filePath + fileName, aiProcess_Triangulate);
  if (!scene)
  {
    throw std::runtime_error("Failed to load model from " + filePath + fileName);
    return nullptr;
  }
  return BuildModel(filePath, scene);
}
}

