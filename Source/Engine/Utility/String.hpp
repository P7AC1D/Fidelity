#pragma once
#include <string>
#include <vector>

#include "../Core/Types.hpp"

class String
{
public:
  static std::string foadFromFile(const std::string &path);
  static std::vector<std::string> split(const std::string &inputLine, byte deliminator);
  static std::string join(const std::vector<std::string> &tokens, byte seperator);
};
