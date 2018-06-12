#pragma once
#include <memory>
#include <unordered_map>
#include <string>

#include "../Core/Types.hpp"

static const std::string RESOURCE_PATH = "./../../Resources/";

class Renderable;

namespace Rendering
{
class Texture;
class TextureCube;
}

class AssetManager
{
public:
  static std::shared_ptr<Rendering::Texture> GetTexture(const std::string& textureName);
  static std::shared_ptr<Rendering::Texture> GetTexture(const std::string& textureFullPath, const std::string& textureName);
  static std::shared_ptr<Rendering::TextureCube> GetTextureCube(const std::string& directory, const std::vector<std::string>& fileNames);
  static std::shared_ptr<Renderable> GetRenderable(const std::string& filePath, const std::string& fileName);

private:
  static std::unordered_map<std::string, std::shared_ptr<Rendering::Texture>> _textureCache;
  static std::unordered_map<std::string, std::shared_ptr<Renderable>> _renderableCache;
};
