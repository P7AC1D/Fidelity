#pragma once
#include <string>
#include <vector>
#include "../Core/Types.hpp"

enum class ShaderType
{
  Vertex,
  Fragment,
  Hull,
  Domain,
  Geometry
};

struct ShaderDesc
{
  ShaderType ShaderType;
  std::string Source;
};

class Shader
{
public:
  Shader(const ShaderDesc &desc) : _desc(desc), _isCompiled(false) {}

  const ShaderDesc &getDesc() const { return _desc; }
  bool isCompiled() const { return _isCompiled; }
  const std::string &getShadeLog() const { return _shaderLog; }

  virtual void compile() = 0;

protected:
  ShaderDesc _desc;
  bool _isCompiled;
  std::string _shaderLog;
};
