#include "String.hpp"

#include <fstream>
#include <sstream>
#include "Assert.hpp"

std::string String::foadFromFile(const std::string &path)
{
  std::fstream fstream;
  fstream.open(path, std::fstream::in);
  if (!fstream.is_open())
  {
    throw std::runtime_error("Failed to open file " + path);
  }

  fstream.seekg(0, fstream.end);
  std::streampos fileLength = fstream.tellg();
  fstream.seekg(0, fstream.beg);

  std::vector<byte> buffer((int32)fileLength);
  fstream.read(&buffer[0], fileLength);

  return std::string(buffer.begin(), buffer.end());
}

std::vector<std::string> String::split(const std::string &inputLine, byte deliminator)
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

std::string String::join(const std::vector<std::string> &tokens, byte seperator)
{
  std::stringstream output;
  for (auto &token : tokens)
  {
    output << token << seperator;
  }
  return output.str();
}
