#pragma once
#include <string>
#include <vector>

#include "../Core/Types.hpp"

class String
{
public:
  static std::string LoadFromFile(const std::string& path);
  static std::vector<std::string> Split(const std::string& inputLine, byte deliminator);
  static std::string Join(const std::vector<std::string>& tokens, byte seperator);
};
