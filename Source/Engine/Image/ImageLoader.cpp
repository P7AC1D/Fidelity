#include "ImageLoader.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stdexcept>

#include "STB/stb_image.h"
#include "../Utility/Assert.hpp"

ImageFormat getFormat(int channelCount)
{
  switch (channelCount)
  {
  case 4:
    return ImageFormat::RGBA8;
  case 3:
    return ImageFormat::RGB8;
  case 2:
    return ImageFormat::RG8;
  case 1:
    return ImageFormat::R8;
  default:
    throw std::runtime_error("Unsupported ImageFormat");
  }
}

std::shared_ptr<ImageData> ImageLoader::loadFromFile(const std::string &filePath)
{
  int32 width;
  int32 height;
  int32 channelCount;
  stbi_set_flip_vertically_on_load(true);
  ubyte *rawData = stbi_load(filePath.c_str(), &width, &height, &channelCount, 0);
  if (!rawData)
  {
    throw std::runtime_error("Could not load image file " + filePath);
  }

  auto imageData = buildImageData(rawData, width, height, channelCount);
  delete[] rawData;
  return imageData;
}

std::shared_ptr<ImageData> ImageLoader::buildImageData(ubyte *data, int32 width, int32 height, int32 channelCount)
{
  ImageFormat format = getFormat(channelCount);
  std::shared_ptr<ImageData> imageData(new ImageData(width, height, 1, format));
  imageData->writeData(data);
  return imageData;
}
