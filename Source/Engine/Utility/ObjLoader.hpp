#pragma once
#include <fstream>
#include <string>

#include "../Core/Types.hpp"
#include "../Maths/Vector2.hpp"
#include "../Maths/Vector3.hpp"

class Model;

namespace Utility
{
class AssetManager;

class ObjLoader
{
public:
  static Model* LoadFromFile(const std::string& filePath, const std::string& fileName, Utility::AssetManager& assetManager);
  
private:
};
}