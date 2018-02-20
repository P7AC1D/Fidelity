#pragma once
#include <memory>
#include <string>

#include "../Overlay/Font.hpp"

class FntLoader
{
public:
  /** Loads a .fnt font from file. Note that only single page fonts are supported.*/
  static std::shared_ptr<Font> LoadFontFromFile(const std::string& fullPath);
};