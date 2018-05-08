#pragma once
#include <memory>
#include <string>
#include "ImageData.hpp"

class ImageLoader
{
public:
  static std::shared_ptr<ImageData> LoadFromFile(const std::string& filePath);
};
