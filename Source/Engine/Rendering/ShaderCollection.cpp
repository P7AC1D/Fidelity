#include "ShaderCollection.h"

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
