#include "StringUtil.h"

std::vector<std::string> StringUtil::Split(const std::string& inputLine, byte deliminator)
{
  std::vector<std::string> output;
  output.reserve(inputLine.size());

  size_t start = 0;
  for (size_t end = 0; end < std::string::npos;)
  {
    end = inputLine.find(deliminator, start);
    output.emplace_back(inputLine.substr(start, end - start));
    start = end + 1;
  }
  return output;
}