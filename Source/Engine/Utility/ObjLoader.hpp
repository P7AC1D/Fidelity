#pragma once
#include <fstream>
#include <string>

#include "../Core/Types.hpp"
#include "../Maths/Vector2.hpp"
#include "../Maths/Vector3.hpp"

class StaticMesh;

struct Face
{
  Vector3 Position;
  Vector3 Normal;
  Vector2 TextureUv;
};

enum ObjLoaderPostProcessing
{
  Generate_Normals = 1 << 0,
  Calculate_Tangent_Space = 1 << 1,
  Generate_Smooth_Normals = 1 << 2,
};

class ObjLoader
{
public:
  static StaticMesh* LoadFromFile(const std::string& filePath, uint32 postProcessingFlags = 0);
  
private:
  static Face ParseLine(const std::string& line);
  static std::string ReadSource(std::ifstream& fileStream);
};
