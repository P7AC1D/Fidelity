#include "ObjLoader.hpp"

#include <exception>
#include <vector>

#include "../Rendering/StaticMesh.h"

StaticMesh* ObjLoader::LoadFromFile(const std::string& filePath, uint32 postProcessingFlags)
{
  std::ifstream fstream;
  fstream.open(filePath, std::fstream::in);
  if (!fstream.is_open())
  {
    throw std::runtime_error("Failed to open file '" + filePath + "'");
  }
  
  const std::vector<Vector3> vertexPositions;
  
  std::string line;
  while (std::getline(fstream, line))
  {
    if (line[0] == 'v')
    {
      
    }
  }
}
  
std::string ObjLoader::ReadSource(std::ifstream& fstream)
{
  fstream.seekg(0, fstream.end);
  std::streampos fileLength = fstream.tellg();
  fstream.seekg(0, fstream.beg);
  
  std::vector<byte> source;
  source.reserve(fileLength);
  fstream.read(source.data(), fileLength);
  return std::string(source.begin(), source.end());
}
