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

std::vector<Colour> BuildPixelDataR8(ubyte* data, int32 width, int32 height)
{
  std::vector<Colour> pixelData(width * height);
  for (uint32 i = 0; i < width * height; i++)
  {
    pixelData[i][0] = static_cast<float32>(data[i]);
  }
  return pixelData;
}

std::vector<Colour> BuildPixelDataRG8(ubyte* data, int32 width, int32 height)
{
  std::vector<Colour> pixelData(width * height);
  for (uint32 i = 0; i < width * height; i += 2)
  {
    pixelData[i][0] = static_cast<float32>(data[i]);
    pixelData[i][1] = static_cast<float32>(data[i + 1]);
  }
  return pixelData;
}

std::vector<Colour> BuildPixelDataRGB8(ubyte* data, int32 width, int32 height)
{
  std::vector<Colour> pixelData(width * height);
  for (uint32 i = 0; i < width * height; i += 3)
  {
    pixelData[i][0] = static_cast<float32>(data[i]);
    pixelData[i][1] = static_cast<float32>(data[i + 1]);
    pixelData[i][2] = static_cast<float32>(data[i + 2]);
  }
  return pixelData;
}

std::vector<Colour> BuildPixelDataRGBA8(ubyte* data, int32 width, int32 height)
{
  std::vector<Colour> pixelData(width * height);
  for (uint32 i = 0; i < width * height; i += 4)
  {
    pixelData[i][0] = static_cast<float32>(data[i]);
    pixelData[i][1] = static_cast<float32>(data[i + 1]);
    pixelData[i][2] = static_cast<float32>(data[i + 2]);
    pixelData[i][2] = static_cast<float32>(data[i + 4]);
  }
  return pixelData;
}

std::vector<Colour> BuildColourArray(ubyte* data, int32 width, int32 height, uint32 channelCount)
{
  Assert::ThrowIfTrue(channelCount > 4, "Channel count exceeds maximum allowed");

  std::vector<Colour> pixelData(width * height);
  for (uint32 i = 0; i < width * height; i += channelCount)
  {
    pixelData[i] = Colour(data[i], data[i + 1], data[i + 2], data[i + 3]);
  }
  return pixelData;
}

std::shared_ptr<ImageData> BuildImageData(ubyte* data, int32 width, int32 height, int32 channelCount)
{
  ImageFormat format = GetFormat(channelCount);
  std::shared_ptr<ImageData> imageData(new ImageData(width, height, 1, format));

  switch (format)
  {
    case ImageFormat::RGBA8:
      imageData->WriteData(BuildPixelDataRGBA8(data, width, height));
      return imageData;
    case ImageFormat::RGB8:
      imageData->WriteData(BuildPixelDataRGB8(data, width, height));
      return imageData;
    case ImageFormat::RG8:
      imageData->WriteData(BuildPixelDataRG8(data, width, height));
      return imageData;
    case ImageFormat::R8:
      imageData->WriteData(BuildPixelDataR8(data, width, height));
      return imageData;
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
