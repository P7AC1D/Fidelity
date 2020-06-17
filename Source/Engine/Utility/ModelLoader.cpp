#include "ModelLoader.hpp"


#include <stdexcept>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "../Maths/Math.hpp"
#include "../Rendering/Material.hpp"
#include "../Rendering/MaterialFactory.hpp"
#include "../Rendering/Renderable.hpp"
#include "../Rendering/StaticMesh.h"
#include "../SceneManagement/ActorNode.hpp"
#include "../SceneManagement/SceneNode.hpp"
#include "../SceneManagement/Transform.h"
#include "Assert.hpp"
#include "String.hpp"
#include "TextureLoader.hpp"
#include "../SceneManagement/GenericNode.hpp"

void OffsetVertices(std::vector<Vector3>& vertices, const Vector3& midPoint)
{
	for (uint32 i = 0; i < vertices.size(); i++)
	{
		vertices[i] = vertices[i] - midPoint;
	}
}

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

Vector3 BuildVertexData(const aiVector3D* vertices, uint32 verexCount, std::vector<Vector3>& verticesOut)
{
	Vector3 min(std::numeric_limits<float32>::max());
	Vector3 max(std::numeric_limits<float32>::min());

  verticesOut.reserve(verexCount);
  for (uint32 i = 0; i < verexCount; i++)
  {
		Vector3 vertex(vertices[i].x, vertices[i].y, vertices[i].z);

		min = Math::Min(min, vertex);
		max = Math::Max(max, vertex);

    verticesOut.push_back(vertex);
  }

	Vector3 midPoint;
	midPoint.X = (max.X + min.X) / 2.0f;
	midPoint.Y = (max.Y + min.Y) / 2.0f;
	midPoint.Z = (max.Z + min.Z) / 2.0f;
	return midPoint;
}

void BuildNormalData(const aiVector3D* normals, uint32 normalCount, std::vector<Vector3>& normalsOut)
{
	normalsOut.reserve(normalCount);
	for (uint32 i = 0; i < normalCount; i++)
	{
		normalsOut.emplace_back(normals[i].x, normals[i].y, normals[i].z);
	}
}

std::shared_ptr<Material> BuildMaterial(const std::string& filePath, const aiMaterial* aiMaterial)
{
	std::shared_ptr<Material> material(MaterialFactory::Create());

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
	return material;
}

std::shared_ptr<StaticMesh> BuildMesh(const std::string& filePath, const aiMesh* aiMesh, bool reconstructWorldTransforms, Vector3& offset)
{
  if (!aiMesh->HasPositions() || !aiMesh->HasNormals())
  {
    return nullptr;
  }
  
  std::shared_ptr<StaticMesh> mesh(new StaticMesh());
  auto material = mesh->GetMaterial();
  
  std::vector<Vector3> vertices;
  Vector3 midPoint = BuildVertexData(aiMesh->mVertices, aiMesh->mNumVertices, vertices);
	if (reconstructWorldTransforms)
	{
		offset = midPoint;
		OffsetVertices(vertices, midPoint);
	}
  mesh->SetPositionVertexData(vertices);
  
  if (aiMesh->HasNormals())
  {
    std::vector<Vector3> normals;
		BuildNormalData(aiMesh->mNormals, aiMesh->mNumVertices, normals);
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

std::shared_ptr<SceneNode> BuildModel(const std::string& fileFolder, const aiScene* scene, bool reconstructWorldTransforms)
{
  if (!scene->HasMeshes() || !scene->HasMaterials())
  {
    return nullptr;
  }

	std::vector<std::shared_ptr<Material>> materials(scene->mNumMaterials);
	for (uint32 i = 0; i < scene->mNumMaterials; i++)
	{		
		materials[i] = BuildMaterial(fileFolder, scene->mMaterials[i]);		
	}
  
  std::shared_ptr<GenericNode> parentNode(SceneNode::Create<GenericNode>(scene->mRootNode->mName.C_Str()));
  for (uint32 i = 0; i < scene->mNumMeshes; i++)
  {
		Vector3 offset;

    auto aiMesh = scene->mMeshes[i];
    auto mesh = BuildMesh(fileFolder, aiMesh, reconstructWorldTransforms, offset);
		mesh->SetMaterial(materials[aiMesh->mMaterialIndex]);
    
    std::shared_ptr<ActorNode> childNode(SceneNode::Create<ActorNode>(aiMesh->mName.C_Str()));
  	
    parentNode->AddChild(childNode);
  	   
    std::shared_ptr<Renderable> renderable(new Renderable());  	
    renderable->SetMesh(mesh);    
    childNode->AddComponent(renderable);
  }
  return parentNode;
}

std::shared_ptr<SceneNode> ModelLoader::LoadFromFile(const std::string& filePath, bool reconstructWorldTransforms)
{
  Assimp::Importer importer;
  auto scene = importer.ReadFile(filePath, aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_GenUVCoords);
  ASSERT_TRUE(scene, "failed to load mode from " + filePath);
  
  auto splitPath = String::Split(filePath, '/');
	splitPath.pop_back();
	auto fileFolder = String::Join(splitPath, '/');
  return BuildModel(fileFolder, scene, reconstructWorldTransforms);
}
