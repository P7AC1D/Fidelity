#pragma once
#include <memory>
#include <unordered_map>
#include <string>

#include "../Core/Types.hpp"

namespace Rendering
{
class Renderable;
class Texture;
class TextureCube;
}

namespace Utility
{
class AssetManager
{
public:
  AssetManager(std::string assetDirectory);
  ~AssetManager();

  std::shared_ptr<Rendering::Texture> GetTexture(const std::string& textureName);
  std::shared_ptr<Rendering::TextureCube> GetTextureCube(const std::string& directory, const std::vector<std::string>& fileNames);
  std::shared_ptr<Rendering::Renderable> GetRenderable(const std::string& filePath, const std::string& fileName);

  AssetManager(AssetManager&) = delete;
  AssetManager(AssetManager&&) = delete;
  AssetManager& operator=(AssetManager&) = delete;
  AssetManager& operator= (AssetManager&&) = delete;

  friend class ObjLoader;

private:
  std::shared_ptr<Rendering::Texture> GetTexture(const std::string& textureFullPath, const std::string& textureName);

private:
  std::string _assetDirectory;
  std::unordered_map<std::string, std::shared_ptr<Rendering::Texture>> _textureCache;
  std::unordered_map<std::string, std::shared_ptr<Rendering::Renderable>> _renderableCache;
};
}