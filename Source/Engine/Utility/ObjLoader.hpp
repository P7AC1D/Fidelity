#pragma once
#include <memory>
#include <string>
#include <vector>

class Model;

namespace Utility
{
class AssetManager;

class ObjLoader
{
public:
  static std::shared_ptr<Model> LoadFromFile(const std::string& filePath, const std::string& fileName, Utility::AssetManager& assetManager);
  
private:
};
}