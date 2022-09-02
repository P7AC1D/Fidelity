#include "ModelLoader.hpp"

#include <stdexcept>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "../Maths/Math.hpp"
#include "../Core/Component.h"
#include "../Core/Transform.h"
#include "../Core/GameObject.h"
#include "../RenderApi/RenderDevice.hpp"
#include "../Rendering/Drawable.h"
#include "../Rendering/Material.h"
#include "../Rendering/StaticMesh.h"
#include "Assert.hpp"
#include "String.hpp"
#include "TextureLoader.hpp"

Vector3 toVector3(aiVector3D input)
{
  return Vector3(input.x, input.y, input.z);
}

void OffsetVertices(std::vector<Vector3> &vertices, const Vector3 &midPoint)
{
  for (uint32 i = 0; i < vertices.size(); i++)
  {
    vertices[i] = vertices[i] - midPoint;
  }
}

void BuildIndexData(const aiFace *faces, uint32 indexCount, std::vector<uint32> &indicesOut)
{
  indicesOut.reserve(indexCount);
  for (uint32 i = 0; i < indexCount; i++)
  {
    auto face = faces + i;
    if (face->mNumIndices != 3)
    {
      throw std::runtime_error("Non-triangle face read");
    }
    indicesOut.push_back(face->mIndices[0]);
    indicesOut.push_back(face->mIndices[1]);
    indicesOut.push_back(face->mIndices[2]);
  }
}

void BuildTexCoordData(const aiVector3D *texCoords, uint32 texCoordCount, std::vector<Vector2> &texCoordsOut)
{
  texCoordsOut.reserve(texCoordCount);
  for (uint32 i = 0; i < texCoordCount; i++)
  {
    texCoordsOut.emplace_back(texCoords[i].x, texCoords[i].y);
  }
}

Vector3 BuildVertexData(const aiVector3D *vertices, uint32 verexCount, std::vector<Vector3> &verticesOut)
{
  Vector3 avg(0);

  verticesOut.reserve(verexCount);
  for (uint32 i = 0; i < verexCount; i++)
  {
    Vector3 vertex(vertices[i].x, vertices[i].y, vertices[i].z);

    avg.X = (avg.X + vertices[i].x) / 2.0f;
    avg.Y = (avg.Y + vertices[i].y) / 2.0f;
    avg.Z = (avg.Z + vertices[i].z) / 2.0f;

    verticesOut.push_back(vertex);
  }

  return avg;
}

void BuildNormalData(const aiVector3D *normals, uint32 normalCount, std::vector<Vector3> &normalsOut)
{
  normalsOut.reserve(normalCount);
  for (uint32 i = 0; i < normalCount; i++)
  {
    normalsOut.emplace_back(normals[i].x, normals[i].y, normals[i].z);
  }
}

std::shared_ptr<Material> BuildMaterial(std::shared_ptr<RenderDevice> renderDevice, const std::string &filePath, const aiMaterial *aiMaterial)
{
  std::shared_ptr<Material> material(new Material());

  aiColor3D ambientColour;
  aiMaterial->Get(AI_MATKEY_COLOR_AMBIENT, ambientColour);
  material->setAmbientColour(Vector3(ambientColour.r, ambientColour.g, ambientColour.b));

  aiColor3D diffuseColour;
  aiMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColour);
  material->setDiffuseColour(Vector3(diffuseColour.r, diffuseColour.g, diffuseColour.b));

  aiColor3D specularColour;
  aiMaterial->Get(AI_MATKEY_COLOR_SPECULAR, specularColour);
  material->setSpecularColour(Vector3(specularColour.r, specularColour.g, specularColour.b));

  float32 specularShininess;
  aiMaterial->Get(AI_MATKEY_SHININESS, specularShininess);
  material->setSpecularExponent(specularShininess);

  if (aiMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0)
  {
    aiString diffuseTexturePath;
    aiMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &diffuseTexturePath);
    if (diffuseTexturePath.length != 0)
    {
      auto diffuseTexture = TextureLoader::LoadFromFile2D(renderDevice, filePath + diffuseTexturePath.C_Str(), true);
      material->setDiffuseTexture(diffuseTexture);
    }
  }

  if (aiMaterial->GetTextureCount(aiTextureType_NORMALS) > 0)
  {
    aiString normalTexturePath;
    aiMaterial->GetTexture(aiTextureType_NORMALS, 0, &normalTexturePath);
    if (normalTexturePath.length != 0)
    {
      auto normalTexture = TextureLoader::LoadFromFile2D(renderDevice, filePath + normalTexturePath.C_Str());
      material->setNormalTexture(normalTexture);
    }
  }

  if (aiMaterial->GetTextureCount(aiTextureType_SPECULAR) > 0)
  {
    aiString specularTexturePath;
    aiMaterial->GetTexture(aiTextureType_SPECULAR, 0, &specularTexturePath);
    if (specularTexturePath.length != 0)
    {
      auto specularTexture = TextureLoader::LoadFromFile2D(renderDevice, filePath + specularTexturePath.C_Str());
      material->setSpecularTexture(specularTexture);
    }
  }
  return material;
}

Vector3 CalculateCentroid(const aiMesh *mesh)
{
  float32 areaSum = 0.0f;
  Vector3 centroid = Vector3::Zero;
  for (int i = 0; i < mesh->mNumFaces; i++)
  {
    auto face = mesh->mFaces[i];
    auto p0 = toVector3(mesh->mVertices[face.mIndices[0]]);
    auto p1 = toVector3(mesh->mVertices[face.mIndices[1]]);
    auto p2 = toVector3(mesh->mVertices[face.mIndices[2]]);

    Vector3 center = (p0 + p1 + p2) / 3.0f;
    float32 area = 0.5f * Vector3::Cross(p1 - p0, p2 - p0).Length();
    centroid += area * center;
    areaSum += area;
  }
  return centroid / areaSum;
}

std::shared_ptr<StaticMesh> BuildMesh(const std::string &filePath, const aiMesh *aiMesh, bool reconstructWorldTransforms, Vector3 &offset)
{
  if (!aiMesh->HasPositions() || !aiMesh->HasNormals())
  {
    return nullptr;
  }

  Vector3 centroid = CalculateCentroid(aiMesh);

  std::shared_ptr<StaticMesh> mesh(new StaticMesh());

  std::vector<Vector3> vertices;
  BuildVertexData(aiMesh->mVertices, aiMesh->mNumVertices, vertices);
  if (reconstructWorldTransforms)
  {
    offset = centroid;
    OffsetVertices(vertices, centroid);
  }
  mesh->setPositionVertexData(vertices);

  if (aiMesh->HasNormals())
  {
    std::vector<Vector3> normals;
    BuildNormalData(aiMesh->mNormals, aiMesh->mNumVertices, normals);
    mesh->setNormalVertexData(normals);
  }
  else
  {
    mesh->generateNormals();
  }

  // Assume that mesh contains a single set of texture coordinate data.
  if (aiMesh->HasTextureCoords(0))
  {
    std::vector<Vector2> texCoords;
    BuildTexCoordData(aiMesh->mTextureCoords[0], aiMesh->mNumVertices, texCoords);
    mesh->setTextureVertexData(texCoords);
  }
  else
  {
    std::vector<Vector2> texCoords(aiMesh->mNumVertices);
    mesh->setTextureVertexData(texCoords);
  }

  if (aiMesh->HasFaces())
  {
    std::vector<uint32> indices;
    BuildIndexData(aiMesh->mFaces, aiMesh->mNumFaces, indices);
    mesh->setIndexData(indices);
  }

  mesh->generateTangents();
  return mesh;
}

GameObject &BuildModel(Scene &scene, const std::string &fileFolder, const aiScene *aiScene, bool reconstructWorldTransforms)
{
  GameObject &root = scene.createGameObject(aiScene->mName.C_Str());

  std::vector<std::shared_ptr<Material>> materials(aiScene->mNumMaterials);
  for (uint32 i = 0; i < aiScene->mNumMaterials; i++)
  {
    materials[i] = BuildMaterial(scene.getRenderDevice(), fileFolder, aiScene->mMaterials[i]);
  }

  for (uint32 i = 0; i < aiScene->mNumMeshes; i++)
  {
    auto aiMesh = aiScene->mMeshes[i];

    GameObject &currentObject = scene.createGameObject(aiMesh->mName.C_Str());
    Drawable &drawable = scene.createComponent<Drawable>();

    currentObject.addComponent(drawable);
    root.addChild(currentObject);

    Vector3 offset;
    drawable.setMaterial(materials[aiMesh->mMaterialIndex]);
    drawable.setMesh(BuildMesh(fileFolder, aiMesh, reconstructWorldTransforms, offset));
    currentObject.transform().setPosition(offset);
  }
}

GameObject &ModelLoader::FromFile(Scene &scene, const std::string &filePath, bool reconstructWorldTransforms)
{
  Assimp::Importer importer;
  auto aiScene = importer.ReadFile(filePath, aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_GenUVCoords);
  ASSERT_TRUE(aiScene, "failed to load mode from " + filePath);

  auto splitPath = String::Split(filePath, '/');
  splitPath.pop_back();
  auto fileFolder = String::Join(splitPath, '/');
  return BuildModel(scene, fileFolder, aiScene, reconstructWorldTransforms);
}
