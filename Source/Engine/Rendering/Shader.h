#pragma once
#include <string>
#include <unordered_map>
#include <vector>

#include "../Core/Types.hpp"
#include "../Maths/Matrix3.hpp"
#include "../Maths/Matrix4.hpp"
#include "../Maths/Vector3.hpp"
#include "../Maths/Vector4.hpp"

namespace Rendering
{
enum class ShaderType
{
  Vertex,
  Fragment
};

enum class ShaderDataType
{
  Bool,
  Float,
  Vec2,
  Vec3,
  Vec4,
  Int,
  Uint,
  Mat3,
  Mat4,
  Sampler2D,
  SamplerCube,
  Unknown,
};

struct ShaderUniform
{
  ShaderUniform(uint32 location, uint32 size, ShaderDataType type, const std::string& name):
    Name(name),
    Location(location),
    Type(type),
    Size(size)
  {}

  std::string Name;
  uint32 Location;
  ShaderDataType Type;
  uint32 Size;
};

struct ShaderUniformBlock
{
  ShaderUniformBlock(const std::string& name, uint32 bindingPoint):
    Name(name),
    BindingPoint(bindingPoint)
  {}

  std::string Name;
  uint32 BindingPoint;
};

class Shader
{
public:
  virtual ~Shader();

protected:
  Shader(const std::string& fileName);

  void SetBool(const std::string& uniformName, bool value);

  void SetInt(const std::string& uniformName, int32 value);
  void SetFloat(const std::string& uniformName, float32 value);

  void SetVec2(const std::string& uniformName, const Vector3& value);
  void SetVec3(const std::string& uniformName, const Vector3& value);
  void SetVec4(const std::string& uniformName, const Vector4& value);
  void SetMat3(const std::string& uniformName, const Matrix3& value);
  void SetMat4(const std::string& uniformName, const Matrix4& value);

  void SetVec3Array(const std::string& uniformName, const std::vector<Vector3>& values);
  void SetVec4Array(const std::string& uniformName, const std::vector<Vector4>& values);

  void BindUniformBlock(int32 location, int32 bindingPoint, int32 ubo);  
  int32 GetUniformBlockBindingPoint(const std::string& name);

  void Bind();
  void Unbind();
  bool IsBound() const;
  
private:
  void LoadFromFile();
  std::string ReadSource(std::fstream& fstream);
  uint32 AttachSource(ShaderType shaderType, const std::string& shaderSource);
  void Compile(uint32 shaderId);
  void AttachShaders(uint32 vertexShaderId, uint32 fragmentShaderId);
  void Link();
  void BuildUniformDeclaration();
  void BuildUniformBufferDeclaration();

  int32 GetUniformLocation(const std::string& name);

private:
  uint32 _programId;
  std::string _fileName;  
  std::unordered_map<std::string, ShaderUniform> _uniforms;
  std::unordered_map<std::string, ShaderUniformBlock> _uniformBlocks;

  friend class RenderApi;
};
}
