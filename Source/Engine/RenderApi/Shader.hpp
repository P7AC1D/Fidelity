#pragma once
#include <string>
#include <vector>
#include "../Core/Types.hpp"

/**
 * @brief Shader stages supported by the renderer.
 */
enum class ShaderType
{
  Vertex,
  Fragment,
  Hull,
  Domain,
  Geometry
};

/**
 * @brief Description used to create a shader from source code.
 */
struct ShaderDesc
{
  ShaderType ShaderType;
  std::string Source;
};

/**
 * @brief Abstract shader object.
 */
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
