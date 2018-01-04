#pragma once

#include <string>
#include <vector>

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#endif
#ifdef _WIN32
#include <GL/glew.h>
#endif

#include "../Core/Types.hpp"
#include "../Maths/Matrix3.hpp"
#include "../Maths/Matrix4.hpp"
#include "../Maths/Vector3.hpp"
#include "../Maths/Vector4.hpp"

namespace Rendering
{
enum class ShaderType : GLenum
{
  Vertex = GL_VERTEX_SHADER,
  Fragment = GL_FRAGMENT_SHADER,
};

class Shader
{
public:
  Shader(const std::string& fileName);
  ~Shader();

  void SetUniformInt(int32 location, int32 value);
  void SetUniformFloat(int32 location, float32 value);

  void SetUniformVec3(int32 location, const Vector3& value);
  void SetUniformVec4(int32 location, const Vector4& value);
  void SetUniformMat3(int32 location, const Matrix3& value);
  void SetUniformMat4(int32 location, const Matrix4& value);

  void SetUniformVec3Array(int32 location, const std::vector<Vector3>& values);
  void SetUniformVec4Array(int32 location, const std::vector<Vector4>& values);

  void BindUniformBlock(int32 location, int32 bindingPoint, int32 ubo, int32 sizeBytes);

  int32 GetUniformLocation(const std::string& name);
  int32 GetAttributeLocation(const std::string& name);
  int32 GetUniformBlockIndex(const std::string& name);

  void Bind();
  void Unbind();
  int32 GetShaderId() const { return m_programId; }

  bool IsBound() const;
  
private:
  void LoadFromFile();
  std::string ReadSource(std::fstream& fstream);
  uint32 AttachSource(ShaderType shaderType, const std::string& shaderSource);
  void Compile(uint32 shaderId);
  void AttachShaders(uint32 vertexShaderId, uint32 fragmentShaderId);
  void Link();  

  uint32 m_programId;

  std::string m_shaderFileName;  
};
}
