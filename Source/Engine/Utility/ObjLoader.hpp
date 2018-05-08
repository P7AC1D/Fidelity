#pragma once
#include <memory>
#include <string>
#include <vector>

class AssetManager;

namespace Rendering
{
class Renderable;
}

namespace Utility
{
class ObjLoader
{
public:
  static std::shared_ptr<Rendering::Renderable> LoadFromFile(const std::string& filePath, const std::string& fileName);
};
}
