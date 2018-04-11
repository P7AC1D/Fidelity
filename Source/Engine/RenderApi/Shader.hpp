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
  std::string Source;
  ShaderLang ShaderLang = ShaderLang::Glsl;
};

class Shader
{
public:
  Shader(): _isCompiled(false) {}
  
  const ShaderDesc& GetDesc() const { return _desc; }
  bool IsCompiled() const { return _isCompiled; }
  const std::string& GetShadeLog() const { return _shaderLog; }
  
  virtual void Compile() = 0;
  
protected:
  ShaderDesc _desc;
  bool _isCompiled;
  std::string _shaderLog;
};
