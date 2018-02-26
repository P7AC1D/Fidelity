#include "AssetManager.h"

#include <array>
#include <cassert>
#include <stdexcept>

#define STB_IMAGE_IMPLEMENTATION
#include "STB/stb_image.h"

#include "../Rendering/OpenGL.h"
#include "../Rendering/Texture.hpp"
#include "../Rendering/TextureCube.hpp"
#include "../Rendering/Renderable.hpp"
#include "../Utility/ObjLoader.hpp"
#include "../Utility/StringUtil.h"
#include "ObjLoader.hpp"

using namespace Utility;
using namespace Rendering;

std::unordered_map<std::string, std::shared_ptr<Texture>> AssetManager::_textureCache;
std::unordered_map<std::string, std::shared_ptr<Renderable>> AssetManager::_renderableCache;

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

PixelFormat CalculateTextureFormat(int32 nChannels)
{
  switch (nChannels)
  {
  case 4: return PixelFormat::RGBA8;
  case 3: return PixelFormat::RGB8;
  case 2: return PixelFormat::RG8;
  case 1: return PixelFormat::R8;
  default: return PixelFormat::RGBA8;
  }
}

size_t GetTextureFaceIndexFromFileName(const std::string& fileName)
{
  auto split = StringUtil::Split(fileName, '.');
  if (split[0] == "back") return 0;
  if (split[0] == "bottom") return 1;
  if (split[0] == "front") return 2;
  if (split[0] == "left") return 3;
  if (split[0] == "right") return 4;
  if (split[0] == "top") return 5;
  throw std::runtime_error("TextureCube file names must be one of the following: 'back, bottom, front, left, right and top'.");
}

std::shared_ptr<Texture> AssetManager::GetTexture(const std::string& textureName)
{
  return GetTexture(RESOURCE_PATH, textureName);
}

std::shared_ptr<TextureCube> AssetManager::GetTextureCube(const std::string& directory, const std::vector<std::string>& fileNames)
{
  assert(fileNames.size() == 6);

  uint32 width = 0;
  uint32 height = 0;
  uint32 channels = 0;
  std::array<ubyte*, 6> pixelData;

  for (size_t i = 0; i < fileNames.size(); i++)
  {
    std::string fullPath = RESOURCE_PATH + directory + fileNames[i];

    int32 currentWidth = 0;
    int32 currentHeight = 0;
    int32 currentChannels = 0;
    pixelData[GetTextureFaceIndexFromFileName(fileNames[i])] = LoadFromFile(fullPath, currentWidth, currentHeight, currentChannels);
    
    if (width != 0 && width != currentWidth)
    {
      throw std::runtime_error("All images in a TextureCube must be of the same width.");
    }
    if (height != 0 && height != currentHeight)
    {
      throw std::runtime_error("All images in a TextureCube must be of the same height.");
    }
    if (channels != 0 && channels != currentChannels)
    {
      throw std::runtime_error("All images in a TextureCube must have the same number of channels.");
    }

    width = currentWidth;
    height = currentHeight;
    channels = currentChannels;
  }

  TextureCubeDesc desc;
  desc.Width = width;
  desc.Height = height;
  desc.Format = CalculateTextureFormat(channels);
  std::shared_ptr<TextureCube> textureCube(new TextureCube(desc));
  textureCube->UploadData(TextureCubeFace::Back, pixelData[0]);
  textureCube->UploadData(TextureCubeFace::Bottom, pixelData[1]);
  textureCube->UploadData(TextureCubeFace::Front, pixelData[2]);
  textureCube->UploadData(TextureCubeFace::Left, pixelData[3]);
  textureCube->UploadData(TextureCubeFace::Right, pixelData[4]);
  textureCube->UploadData(TextureCubeFace::Top, pixelData[5]);

  for (size_t i = 0; i < pixelData.size(); i++)
  {
    delete[] pixelData[i];
  }
  return textureCube;
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
  auto renderable = ObjLoader::LoadFromFile(filePath, fileName);
  _renderableCache[fullPath] = renderable;
  return renderable;
}

std::shared_ptr<Rendering::Texture> AssetManager::GetTexture(const std::string& texturePath, 
                                                             const std::string& textureName)
{
  auto fullPath = texturePath + textureName;
  auto iter = _textureCache.find(fullPath);
  if (iter == _textureCache.end())
  {
    int32 width = 0;
    int32 height = 0;
    int32 nChannels = 0;
    ubyte* data = LoadFromFile(fullPath, width, height, nChannels);

    TextureDesc desc;
    desc.Width = static_cast<uint32>(width);
    desc.Height = static_cast<uint32>(height);
    desc.Format = CalculateTextureFormat(nChannels);
    std::shared_ptr<Texture> texture(new Texture(desc, data));
    _textureCache.emplace(fullPath, texture);

    delete[] data;
  }
  return _textureCache[fullPath];
}
