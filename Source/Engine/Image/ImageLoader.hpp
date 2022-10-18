#pragma once
#include <memory>
#include <string>
#include "ImageData.hpp"

class ImageLoader
{
public:
  static std::shared_ptr<ImageData> loadFromFile(const std::string &filePath);

private:
  static std::shared_ptr<ImageData> buildImageData(ubyte *data, int32 width, int32 height, int32 channelCount);
};
