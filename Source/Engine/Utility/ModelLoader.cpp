#include "ModelLoader.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "../Rendering/Material.hpp"
#include "../Rendering/Renderable.hpp"
#include "../Rendering/StaticMesh.h"
#include "../SceneManagement/Actor.hpp"
#include "../SceneManagement/SceneNode.hpp"
#include "Assert.hpp"
#include "String.hpp"
#include "TextureLoader.hpp"

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
    indicesOut.push_back(face->mIndices[0]);
    indicesOut.push_back(face->mIndices[1]);
    indicesOut.push_back(face->mIndices[2]);
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

void BuildMaterial(const std::string& filePath, const aiMaterial* aiMaterial, std::shared_ptr<Material> material)
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
  material->SetSpecularExponent(specularShininess);
  
  aiString diffuseTexturePath;
  aiMaterial->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), diffuseTexturePath);
  if (diffuseTexturePath.length != 0)
  {
    auto diffuseTexture = TextureLoader::LoadFromFile2D(filePath + diffuseTexturePath.C_Str(), true);
    material->SetDiffuseTexture(diffuseTexture);
  }
  
  // Assimp for some reason loads normal maps as aiTextureType_HEIGHT - this is probably a bug.
  aiString normalTexturePath;
  aiMaterial->Get(AI_MATKEY_TEXTURE(aiTextureType_HEIGHT, 0), normalTexturePath);
  if (normalTexturePath.length != 0)
  {
    auto normalTexture = TextureLoader::LoadFromFile2D(filePath + normalTexturePath.C_Str());
    material->SetNormalTexture(normalTexture);
  }
  
  aiString specularTexturePath;
  aiMaterial->Get(AI_MATKEY_TEXTURE(aiTextureType_SPECULAR, 0), specularTexturePath);
  if (specularTexturePath.length != 0)
  {
    auto specularTexture = TextureLoader::LoadFromFile2D(filePath + specularTexturePath.C_Str());
    material->SetSpecularTexture(specularTexture);
  }
}

std::shared_ptr<StaticMesh> BuildMesh(const std::string& filePath, const aiMesh* aiMesh, const aiMaterial* aiMaterial)
{
  if (!aiMesh->HasPositions() || !aiMesh->HasNormals())
  {
    return nullptr;
  }
  
  std::shared_ptr<StaticMesh> mesh(new StaticMesh());
  auto material = mesh->GetMaterial();
  BuildMaterial(filePath, aiMaterial, material);
  
  std::vector<Vector3> vertices;
  BuildVertexData(aiMesh->mVertices, aiMesh->mNumVertices, vertices);
  mesh->SetPositionVertexData(vertices);
  
  if (aiMesh->HasNormals())
  {
    std::vector<Vector3> normals;
    BuildVertexData(aiMesh->mNormals, aiMesh->mNumVertices, normals);
    mesh->SetNormalVertexData(normals);
  }
  else
  {
    mesh->GenerateNormals();
  }
  
  // Assume that mesh contains a single set of texture coordinate data.
  if (aiMesh->HasTextureCoords(0))
  {
    std::vector<Vector2> texCoords;
    BuildTexCoordData(aiMesh->mTextureCoords[0], aiMesh->mNumVertices, texCoords);
    mesh->SetTextureVertexData(texCoords);
  }
  else
  {
    std::vector<Vector2> texCoords(aiMesh->mNumVertices);
    mesh->SetTextureVertexData(texCoords);
  }
  
  if (aiMesh->HasFaces())
  {
    std::vector<uint32> indices;
    BuildIndexData(aiMesh->mFaces, aiMesh->mNumFaces, indices);
    mesh->SetIndexData(indices);
  }
  
  mesh->GenerateTangents();
  return mesh;
}

std::shared_ptr<SceneNode> BuildModel(const std::string& fileFolder, const aiScene* scene)
{
  if (!scene->HasMeshes() || !scene->HasMaterials())
  {
    return nullptr;
  }
  
  std::shared_ptr<SceneNode> sceneNode(new SceneNode(scene->mRootNode->mName.C_Str()));
  for (uint32 i = 0; i < scene->mNumMeshes; i++)
  {
    auto aiMesh = scene->mMeshes[i];
    auto mesh = BuildMesh(fileFolder, aiMesh, scene->mMaterials[aiMesh->mMaterialIndex]);
    
    std::shared_ptr<Actor> actor(sceneNode->CreateActor(aiMesh->mName.C_Str()));
    std::shared_ptr<Renderable> renderable(new Renderable());
    renderable->SetMesh(mesh);
    
    actor->AddComponent<Renderable>(renderable);
  }
  return sceneNode;
}

std::shared_ptr<SceneNode> ModelLoader::LoadFromFile(const std::string& filePath)
{
  Assimp::Importer importer;
  auto scene = importer.ReadFile(filePath, aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_GenUVCoords);
  ASSERT_TRUE(scene, "failed to load mode from " + filePath);
  
  auto splitPath = String::Split(filePath, '/');
	splitPath.pop_back();
	auto fileFolder = String::Join(splitPath, '/');
  return BuildModel(fileFolder, scene);
}
