#pragma once
#include <string>
#include <vector>
#include "../Core/Types.hpp"

enum class ShaderLang
{
  Hlsl,
  Glsl
};

enum class ShaderType
{
  Vertex,
  Pixel,
  Hull,
  Domain,
  Geometry
};

struct ShaderDesc
{
  ShaderType ShaderType;
  std::string EntryPoint;
  std::vector<byte> Source;
  ShaderLang ShaderLang = ShaderLang::Glsl;
};

class Shader
{
public:
  const ShaderDesc& GetDesc() const { return _desc; }
  
protected:
  ShaderDesc _desc;
};
