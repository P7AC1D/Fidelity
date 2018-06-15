#include "ImageLoader.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "STB/stb_image.h"
#include "../Utility/Assert.hpp"

ImageFormat GetFormat(int channelCount)
{
  switch (channelCount)
  {
    case 4: return ImageFormat::RGBA8;
    case 3: return ImageFormat::RGB8;
    case 2: return ImageFormat::RG8;
    case 1: return ImageFormat::R8;
    default: throw std::runtime_error("Unsupported ImageFormat");
  }
}

std::shared_ptr<ImageData> ImageLoader::LoadFromFile(const std::string& filePath)
{
  int32 width;
  int32 height;
  int32 channelCount;
  ubyte* imageData = stbi_load(filePath.c_str(), &width, &height, &channelCount, 0);
  Assert::ThrowIfFalse(imageData, "Could not load image file " + filePath);
  
  return BuildImageData(imageData, width, height, channelCount);
}

std::shared_ptr<ImageData> ImageLoader::BuildImageData(ubyte* data, int32 width, int32 height, int32 channelCount)
{
	ImageFormat format = GetFormat(channelCount);
	std::shared_ptr<ImageData> imageData(new ImageData(width, height, 1, format));
	imageData->WriteData(data);
	return imageData;
}