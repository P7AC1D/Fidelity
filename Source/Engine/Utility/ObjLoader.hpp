#pragma once
#include <memory>
#include <string>
#include <vector>

namespace Rendering
{
class Renderable;
}

namespace Utility
{
class AssetManager;

class ObjLoader
{
public:
  static std::shared_ptr<Rendering::Renderable> LoadFromFile(const std::string& filePath, const std::string& fileName, Utility::AssetManager& assetManager);
};
}