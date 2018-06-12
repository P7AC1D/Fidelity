#pragma once
#include <memory>
#include <string>
#include <vector>

class AssetManager;
class Renderable;

namespace Utility
{
class ObjLoader
{
public:
  static std::shared_ptr<Renderable> LoadFromFile(const std::string& filePath, const std::string& fileName);
};
}
