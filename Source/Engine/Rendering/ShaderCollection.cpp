#include "ShaderCollection.h"

#include <exception>

#include "../Rendering/Shader.h"

namespace Rendering
{
ShaderCollection::ShaderCollection(std::string shaderDirectory) :
  _shaderDirectory(std::move(shaderDirectory))
{
}

ShaderCollection::~ShaderCollection()
{
}

std::shared_ptr<Shader> ShaderCollection::GetShader(const std::string& shaderName)
{
  auto iter = _shaderCollection.find(shaderName);
  if (iter != _shaderCollection.end())
  {
    return iter->second;
  }
  return LoadShader(shaderName);
}

bool ShaderCollection::HasShader(const std::string& shaderName) const
{
  return _shaderCollection.find(shaderName) != _shaderCollection.end();
}

std::shared_ptr<Shader> ShaderCollection::LoadShader(const std::string& shaderName)
{
  try
  {
    auto shader = std::make_shared<Shader>(_shaderDirectory + shaderName);
    _shaderCollection[shaderName] = shader;
    return shader;
  }
  catch (std::exception exception)
  {
    // LOG_ERROR << "Failed to load shader " << shaderName << ": " << exception.what();
    return nullptr;
  }
}
}
