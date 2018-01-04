#pragma once
#include <memory>
#include <unordered_map>
#include <string>

#include "../Core/Types.hpp"

namespace Rendering
{
enum class TextureFormat;
class CubeMap;
class Texture;
}

namespace Utility
{
class AssetManager
{
public:
  AssetManager(std::string assetDirectory);
  ~AssetManager();

  std::shared_ptr<Rendering::Texture> GetTexture(const std::string& textureName, bool gammaCorrection = false);
  std::shared_ptr<Rendering::CubeMap> GetCubeMap(const std::vector<std::string>& textureNames);

private:
  AssetManager(AssetManager&) = delete;
  AssetManager(AssetManager&&) = delete;
  AssetManager& operator=(AssetManager&) = delete;
  AssetManager& operator= (AssetManager&&) = delete;

  std::string _assetDirectory;
  std::unordered_map<std::string, std::shared_ptr<Rendering::Texture>> _textureCache;
};
}