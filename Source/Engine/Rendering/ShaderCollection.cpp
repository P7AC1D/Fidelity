#include "ShaderCollection.h"

#include <iostream>
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
}
