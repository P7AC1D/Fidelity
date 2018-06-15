#pragma once
#include <memory>
#include <string>
#include "ImageData.hpp"

class ImageLoader
{
public:
  static std::shared_ptr<ImageData> LoadFromFile(const std::string& filePath);

private:
	static std::shared_ptr<ImageData> BuildImageData(ubyte* data, int32 width, int32 height, int32 channelCount);
};
