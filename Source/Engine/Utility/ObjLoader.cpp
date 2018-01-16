#include "ObjLoader.hpp"

#include <algorithm>
#include <fstream>
#include <exception>
#include <memory>
#include <vector>

#include "../Components/Model.hpp"
#include "../Maths/Vector2.hpp"
#include "../Maths/Vector3.hpp"
#include "../Rendering/Material.h"
#include "../Rendering/StaticMesh.h"
#include "../Utility/AssetManager.h"
#include "../Utility/StringUtil.h"

using namespace Rendering;

namespace Utility
{
struct MaterialData
{
  MaterialData(const std::string& name) :
    Name(name)
  {
  }

  std::string Name;
  float32 SpecularExponent;
  Vector3 AmbientColour;
  Vector3 DiffuseColour;
  Vector3 SpecularColour;
  Vector3 EmittanceColour;
  std::string MapKd;
  std::string MapKn;
};

void BuildVertexFromIndexTriplet(const std::string& line, const std::vector<Vector3>& positions,
                                 const std::vector<Vector3>& normals, const std::vector<Vector2>& texCoords,
                                 std::vector<Vector3>& positionsOut, std::vector<Vector3>& normalsOut,
                                 std::vector<Vector2>& texCoordsOut)
{
  auto vertexTokens = StringUtil::Split(line, ' ');
  if (vertexTokens.size() > 4)
  {
    throw std::runtime_error("Only triangulated faces are supported.");
  }

  for (uint32 i = 1; i < 4; i++)
  {
    auto indexToken = StringUtil::Split(vertexTokens[i], '/');
    auto indexTokenCount = indexToken.size();
    if (indexTokenCount == 1) // v
    {
      positionsOut.push_back(positions[std::stoi(indexToken[0]) - 1]);
    }
    else if (indexTokenCount == 2) // v/vt
    {
      positionsOut.push_back(positions[std::stoi(indexToken[0]) - 1]);
      texCoordsOut.push_back(texCoords[std::stoi(indexToken[1]) - 1]);
    }
    else if (indexTokenCount == 3)
    {
      if (indexToken[1] == "") // v//vn
      {
        positionsOut.push_back(positions[std::stoi(indexToken[0]) - 1]);
        normalsOut.push_back(normals[std::stoi(indexToken[2]) - 1]);
      }
      else // v/vt/vn
      {
        positionsOut.push_back(positions[std::stoi(indexToken[0]) - 1]);
        texCoordsOut.push_back(texCoords[std::stoi(indexToken[1]) - 1]);
        normalsOut.push_back(normals[std::stoi(indexToken[2]) - 1]);
      }
    }
    else
    {
      throw std::runtime_error("Face vertices must only contain either positions, normals or texture-coordinates.");
    }
  }
}

std::vector<std::shared_ptr<MaterialData>> LoadMaterialsFromFile(std::ifstream& fstream)
{
  std::vector<std::shared_ptr<MaterialData>> materials;
  std::shared_ptr<MaterialData> material;

  std::string line;
  while (std::getline(fstream, line))
  {
    auto tokens = StringUtil::Split(line, ' ');
    if (tokens[0] == "#" && tokens[1] == "Material" && tokens[2] == "Count:")
    {
      materials.reserve(std::stoi(tokens[3]));
    }
    else if (tokens[0] == "newmtl")
    {
      material.reset(new MaterialData(tokens[1]));      
      materials.push_back(material);
    }
    else if (tokens[0] == "Ns")
    {
      material->SpecularExponent = std::stof(tokens[1]);
    }
    else if (tokens[0] == "Ka")
    {
      material->AmbientColour = Vector3(std::stof(tokens[1]), std::stof(tokens[2]), std::stof(tokens[3]));
    }
    else if (tokens[0] == "Kd")
    {
      material->DiffuseColour = Vector3(std::stof(tokens[1]), std::stof(tokens[2]), std::stof(tokens[3]));
    }
    else if (tokens[0] == "Ks")
    {
      material->SpecularColour = Vector3(std::stof(tokens[1]), std::stof(tokens[2]), std::stof(tokens[3]));
    }
    else if (tokens[0] == "Ke")
    {
      material->EmittanceColour = Vector3(std::stof(tokens[1]), std::stof(tokens[2]), std::stof(tokens[3]));
    }
    else if (tokens[0] == "map_Kd")
    {
      material->MapKd = tokens[1];
    }
    else
    {
      // Ignore other types
    }    
  }
  return materials;
}

std::shared_ptr<Model> ObjLoader::LoadFromFile(const std::string& filePath, const std::string& fileName, Utility::AssetManager& assetManager)
{
  std::ifstream fstream;
  fstream.open(filePath + fileName, std::fstream::in);
  if (!fstream.is_open())
  {
    throw std::runtime_error("Failed to open file '" + filePath + "'");
  }
    
  std::shared_ptr<Model> activeModel = nullptr;
  std::shared_ptr<StaticMesh> activeMesh = nullptr;
  std::shared_ptr<Material> activeMaterial = nullptr;

  std::vector<std::shared_ptr<MaterialData>> materials;
  std::vector<Model> models;
  models.reserve(4);
  
  std::vector<Vector3> positions;
  std::vector<Vector3> normals;
  std::vector<Vector2> texCoords;

  std::vector<Vector3> positionsOut;
  std::vector<Vector3> normalsOut;
  std::vector<Vector2> texCoordsOut;
  
  std::string line;
  while (std::getline(fstream, line))
  {
    auto tokens = StringUtil::Split(line, ' ');
    if (tokens[0] == "o")
    {
      if (activeModel != nullptr)
      {
        positions.clear();
        normals.clear();
        texCoords.clear();

        activeMesh->SetPositionVertexData(positionsOut);
        activeMesh->SetNormalVertexData(normalsOut);
        activeMesh->SetTextureVertexData(texCoordsOut);        
        activeModel->PushMesh(*activeMesh);
        models.push_back(*activeModel);

        positionsOut.clear();
        normalsOut.clear();
        texCoordsOut.clear();
      }
      activeModel.reset(new Model);
    }
    else if (tokens[0] == "mtllib")
    {
      std::ifstream matFile;
      std::string matFilePath = filePath + &line[7];
      matFile.open(matFilePath, std::fstream::in);
      if (!matFile.is_open())
      {
        throw std::runtime_error("Failed to open file '" + matFilePath + "'");
      }
      materials = LoadMaterialsFromFile(matFile);
    }
    else if (tokens[0] == "v")
    {
      positions.emplace_back(std::stof(tokens[1]), std::stof(tokens[2]), std::stof(tokens[3]));
    }
    else if (tokens[0] == "vn")
    {
      normals.emplace_back(std::stof(tokens[1]), std::stof(tokens[2]), std::stof(tokens[3]));
    }
    else if (tokens[0] == "vt")
    {
      texCoords.emplace_back(std::stof(tokens[1]), std::stof(tokens[2]));
    }
    else if (tokens[0] == "usemtl")
    {
      auto iter = std::find_if(materials.begin(), materials.end(), [&](const std::shared_ptr<MaterialData>& material)
      {
        return material->Name == tokens[1];
      });
      if (iter == materials.end())
      {
        throw std::runtime_error("Could not find a material with name " + tokens[1]);
      }

      if (activeMesh != nullptr)
      {
        activeMesh->SetPositionVertexData(positionsOut);
        activeMesh->SetNormalVertexData(normalsOut);
        activeMesh->SetTextureVertexData(texCoordsOut);
        activeModel->PushMesh(*activeMesh);

        positionsOut.clear();
        normalsOut.clear();
        texCoordsOut.clear();
      }

      activeMesh.reset(new StaticMesh(tokens[1]));
      auto material = activeMesh->GetMaterial();
      material->SetAmbientColour((*iter)->AmbientColour);
      material->SetDiffuseColour((*iter)->DiffuseColour);
      material->SetSpecularColour((*iter)->SpecularColour);
      material->SetSpecularShininess((*iter)->SpecularExponent);
      if ((*iter)->MapKd != "")
      {
        material->SetTexture("DiffuseMap", assetManager.GetTexture(filePath, (*iter)->MapKd));
      }
    }
    else if (tokens[0] == "f")
    {
      BuildVertexFromIndexTriplet(line, positions, normals, texCoords, positionsOut, normalsOut, texCoordsOut);
    }
    else if (tokens[0] == "#")
    {
      // do nothing
    }
  }

  activeMesh->SetPositionVertexData(positionsOut);
  activeMesh->SetNormalVertexData(normalsOut);
  activeMesh->SetTextureVertexData(texCoordsOut);
  activeModel->PushMesh(*activeMesh);
  models.push_back(*activeModel);
  std::shared_ptr<Model> model(new Model(models[0]));
  return model;
}
}

