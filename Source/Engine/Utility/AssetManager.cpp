#include "AssetManager.h"

#include <stdexcept>

#define STB_IMAGE_IMPLEMENTATION
#include "STB/stb_image.h"

#include "../Rendering/CubeMap.h"
#include "../Rendering/OpenGL.h"
#include "../Rendering/Texture.h"
#include "../Rendering/Renderable.hpp"
#include "../Utility/ObjLoader.hpp"
#include "ObjLoader.hpp"

using namespace Rendering;

namespace Utility
{
void FlipImage(uint8* imageData, int32 xSize, int32 ySize, int32 nChannels)
{
  int32 widthBytes = xSize * nChannels;
  uint8* top = nullptr;
  uint8* bottom = nullptr;
  uint8 temp = 0;
  int32 halfHeight = ySize / 2;

  for (int32 row = 0; row < halfHeight; ++row)
  {
    top = imageData + row * widthBytes;
    bottom = imageData + (ySize - row - 1) * widthBytes;
    for (int32 col = 0; col < widthBytes; ++col)
    {
      temp = *top;
      *top = *bottom;
      *bottom = temp;
      ++top;
      ++bottom;
    }
  }
}

uint8* LoadFromFile(const std::string& filePath, int32& widthOut, int32& heightOut, int32& nChannels)
{
  uint8* imageData = stbi_load(filePath.c_str(), &widthOut, &heightOut, &nChannels, 0);
  if (!imageData)
  {
    throw std::runtime_error("Failed to load texture from file " + filePath);
  }

  //FlipImage(imageData, widthOut, heightOut, nChannels);
  return imageData;
}

PixelFormat CalculateTextureFormat(int32 nChannels, bool gammaCorrection)
{
  switch (nChannels)
  {
  case 4:
    return gammaCorrection ? PixelFormat::SRGBA : PixelFormat::RGBA;
  case 3:
    return gammaCorrection ? PixelFormat::SRGB : PixelFormat::RGB;
  case 1:
    return PixelFormat::R8;
  default: throw std::runtime_error("Unsupported texture format.");
  }
}

AssetManager::AssetManager(std::string assetDirectory) :
  _assetDirectory(std::move(assetDirectory))
{
}

AssetManager::~AssetManager()
{
}

std::shared_ptr<Texture> AssetManager::GetTexture(const std::string& textureName, bool gammaCorrection)
{
  return GetTexture(_assetDirectory, textureName, gammaCorrection);
}

std::shared_ptr<CubeMap> AssetManager::GetCubeMap(const std::vector<std::string>& textureNames)
{
  auto cubeMap = std::make_shared<CubeMap>();
  glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap->_id);

  for (uint32 i = 0; i < textureNames.size(); ++i)
  {
    auto fullPath = _assetDirectory + textureNames[i];

    int32 width = 0;
    int32 height = 0;
    int32 nChannels = 0;
    uint8* data = LoadFromFile(fullPath, width, height, nChannels);    
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    delete[] data;
  }

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
  return cubeMap;
}

std::shared_ptr<Renderable> AssetManager::GetRenderable(const std::string& filePath, const std::string& fileName)
{
  auto fullPath = filePath + fileName;
  auto iter = _renderableCache.find(fullPath);
  if (iter != _renderableCache.end())
  {
    return iter->second;
  }
  
  //auto renderable = ObjLoader::LoadFromFile(filePath, fileName, *this);
  auto renderable = ObjLoader::LoadFromFile(filePath, fileName, *this);
  _renderableCache[fullPath] = renderable;
  return renderable;
}

std::shared_ptr<Rendering::Texture> AssetManager::GetTexture(const std::string& texturePath, 
                                                             const std::string& textureName,
                                                             bool gammaCorrection)
{
  auto fullPath = texturePath + textureName;
  auto iter = _textureCache.find(fullPath);
  if (iter == _textureCache.end())
  {
    int32 width = 0;
    int32 height = 0;
    int32 nChannels = 0;
    ubyte* data = LoadFromFile(fullPath, width, height, nChannels);

    std::shared_ptr<Texture> texture(new Texture(CalculateTextureFormat(nChannels, gammaCorrection), width, height, data));
    texture->SetMinFilter(TextureMinFilter::Linear);
    texture->SetMagFilter(TextureMagFilter::Linear);
    texture->SetWrapMethod(TextureWrapMethod::ClampToEdge);
    _textureCache.emplace(fullPath, texture);

    delete[] data;
  }
  return _textureCache[fullPath];
}
}
