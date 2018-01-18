#pragma once
#include <string>
#include <vector>

#include "../Core/Types.hpp"

class StringUtil
{
public:
  static std::vector<std::string> Split(const std::string& inputLine, byte deliminator);
};