#include "FntLoader.hpp"

#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "../Utility/StringUtil.h"

std::string ParseStringValue(const std::string& line, const std::string& attributeName)
{
  size_t startPos = line.find(attributeName + "=\"");
  if (startPos == std::string::npos)
  {
    throw std::runtime_error("Could not find attribute " + attributeName + " contained in line:\n" + line);
  }
  
  startPos += attributeName.size() + 2;
  size_t endPos = line.find('\"', startPos);

  if (startPos == std::string::npos)
  {
    throw std::runtime_error("Could not find attribute " + attributeName + " contained in line:\n" + line);
  }
  return line.substr(startPos, endPos - startPos);
}

int32 ParseIntegerValue(const std::string& line, const std::string& attributeName)
{
  size_t startPos = line.find(attributeName + "=");
  if (startPos == std::string::npos)
  {
    throw std::runtime_error("Could not find attribute " + attributeName + " contained in line:\n" + line);
  }
  
  startPos += attributeName.size() + 1;
  size_t endPos = line.find(' ', startPos);
  return std::stoi(line.substr(startPos, endPos - startPos));
}

void ParseCommon(const std::shared_ptr<Font>& font, const std::string& line)
{
  auto tokens = StringUtil::Split(line, ' ');
  for (auto& token : tokens)
  {
    if (token.find("lineHeight") != std::string::npos)
    {
      font->LineHeight = ParseIntegerValue(token, "lineHeight");
    }
    else if (token.find("base") != std::string::npos)
    {
      font->Base = ParseIntegerValue(token, "base");
    }
    else if (token.find("scaleW") != std::string::npos)
    {
      font->TextureWidth = ParseIntegerValue(token, "scaleW");
    }
    else if (token.find("scaleH") != std::string::npos)
    {
      font->TextureHeight = ParseIntegerValue(token, "scaleH");
    }
  }
}

void ParseCharacter(const std::shared_ptr<Font>& font, const std::string& line)
{
  Character character;
  auto tokens = StringUtil::Split(line, ' ');
  for (auto& token : tokens)
  {
    auto attributeValuePair = StringUtil::Split(token, '=');
    if (attributeValuePair[0] == "id")
    {
      character.Id = std::stoi(attributeValuePair[1]);
    }
    else if (attributeValuePair[0] == "x")
    {
      character.XPos = std::stoi(attributeValuePair[1]);
    }
    else if (attributeValuePair[0] == "y")
    {
      character.YPos = std::stoi(attributeValuePair[1]);
    }
    else if (attributeValuePair[0] == "width")
    {
      character.Width = std::stoi(attributeValuePair[1]);
    }
    else if (attributeValuePair[0] == "height")
    {
      character.Height = std::stoi(attributeValuePair[1]);
    }
    else if (attributeValuePair[0] == "xoffset")
    {
      character.XOffset = std::stoi(attributeValuePair[1]);
    }
    else if (attributeValuePair[0] == "yoffset")
    {
      character.YOffset = std::stoi(attributeValuePair[1]);
    }
    else if (attributeValuePair[0] == "xadvance")
    {
      character.XAdvance = std::stoi(attributeValuePair[1]);
    }
  }
  font->Characters.emplace_back(character);
}

void ParseInfo(const std::shared_ptr<Font>& font, const std::string& line)
{
  font->Name = ParseStringValue(line, "face");
  font->Size = ParseIntegerValue(line, "size");
}

void ParseLine(const std::shared_ptr<Font>& font, const std::string& line)
{
  auto tokens = StringUtil::Split(line, ' ');
  if (tokens[0] == "info")
  {
    ParseInfo(font, line);
  }
  else if (tokens[0] == "common")
  {
    ParseCommon(font, line);
  }
  else if (tokens[0] == "page")
  {
    font->TextureFileName = ParseStringValue(line, "file");
  }
  else if (tokens[0] == "chars")
  {
    font->Count = ParseIntegerValue(line, "count");
    font->Characters.reserve(font->Count);
  }
  else if (tokens[0] == "char")
  {
    ParseCharacter(font, line);
  }
}

std::shared_ptr<Font> Parse(std::ifstream& fstream)
{
  std::string line;
  auto font = std::make_shared<Font>();
  while (std::getline(fstream, line))
  {
    ParseLine(font, line);
  }
  return font;
}

std::shared_ptr<Font> FntLoader::LoadFontFromFile(const std::string& fullPath)
{
  try
  {
    std::ifstream fstream;
    fstream.open(fullPath, std::fstream::in);
    if (!fstream.is_open())
    {
      throw std::runtime_error("Could not open file.");
    }
    return Parse(fstream);
  }
  catch (const std::exception& exception)
  {
    throw std::runtime_error("Failed to load font from '" + fullPath + "': " + exception.what());
  }
}
