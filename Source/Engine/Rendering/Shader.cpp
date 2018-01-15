#include "Shader.h"

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#endif
#ifdef _WIN32
#include <GL/glew.h>
#endif

#include <algorithm>
#include <exception>
#include <fstream>
#include <list>
#include <sstream>

namespace Rendering
{
static uint32 s_activeShader = 0;

ShaderType ToShaderType(GLenum shaderType)
{
  switch (shaderType)
  {
    case GL_VERTEX_SHADER: return ShaderType::Vertex;
    case GL_FRAGMENT_SHADER: return ShaderType::Fragment;
    default: throw std::runtime_error("Shader type conversion failed - unknown GLenum.");
  }
}

GLenum FromShaderType(ShaderType shaderType)
{
  switch (shaderType)
  {
    case ShaderType::Vertex: return GL_VERTEX_SHADER;
    case ShaderType::Fragment: return GL_FRAGMENT_SHADER;
    default: throw std::runtime_error("Shader type conversion failed - unknown ShaderType.");
  }
}

ShaderDataType ToShaderDataType(GLenum type)
{
  switch (type)
  {
    case GL_FLOAT: return ShaderDataType::Float;
    case GL_FLOAT_VEC2: return ShaderDataType::Vec2;
    case GL_FLOAT_VEC3: return ShaderDataType::Vec3;
    case GL_FLOAT_VEC4: return ShaderDataType::Vec4;
    case GL_INT: return ShaderDataType::Int;
    case GL_UNSIGNED_INT: return ShaderDataType::Uint;
    case GL_FLOAT_MAT3: return ShaderDataType::Mat3;
    case GL_FLOAT_MAT4: return ShaderDataType::Mat4;
    case GL_SAMPLER_2D: return ShaderDataType::Sampler2D;
    case GL_SAMPLER_CUBE: return ShaderDataType::SamplerCube;
    default: return ShaderDataType::Unknown;
  }
}

Shader::Shader(const std::string& fileName) :
  _fileName(fileName)
{
  _programId = glCreateProgram();
  if (_programId == 0)
  {
    std::stringstream message;
    message << "OpenGL was unable to create a shader program object for shader " << _fileName;
    throw std::runtime_error(message.str());
  }
  LoadFromFile();
  BuildUniformDeclaration();
}

Shader::~Shader()
{
  glDeleteProgram(_programId);
  _programId = 0;
}

void Shader::SetInt(const std::string& uniformName, int32 value)
{
  auto location = GetUniformLocation(uniformName);
  Bind();
  glUniform1i(location, value);
}

void Shader::SetFloat(const std::string& uniformName, float32 value)
{
  auto location = GetUniformLocation(uniformName);
  glUniform1f(location, value);
}

void Shader::SetVec3(const std::string& uniformName, const Vector3& value)
{
  auto location = GetUniformLocation(uniformName);
  Bind();
  glUniform3f(location, value[0], value[1], value[2]);
}

void Shader::SetVec4(const std::string& uniformName, const Vector4& value)
{
  auto location = GetUniformLocation(uniformName);
  Bind();
  glUniform4f(location, value[0], value[1], value[2], value[3]);
}

void Shader::SetMat3(const std::string& uniformName, const Matrix3& value)
{
  auto location = GetUniformLocation(uniformName);
  Bind();
  glUniformMatrix3fv(location, 1, GL_TRUE, &value[0][0]);
}

void Shader::SetMat4(const std::string& uniformName, const Matrix4& value)
{
  auto location = GetUniformLocation(uniformName);
  Bind();
  glUniformMatrix4fv(location, 1, GL_TRUE, &value[0][0]);
}

void Shader::SetVec3Array(const std::string& uniformName, const std::vector<Vector3>& values)
{
  auto location = GetUniformLocation(uniformName);
  Bind();
  glUniform3fv(location, static_cast<int32>(values.size()), values[0].Ptr());
}

void Shader::SetVec4Array(const std::string& uniformName, const std::vector<Vector4>& values)
{
  auto location = GetUniformLocation(uniformName);
  Bind();
  glUniform4fv(location, static_cast<int32>(values.size()), values[0].Ptr());
}

void Shader::BindUniformBlock(int32 location, int32 bindingPoint, int32 ubo, int32 sizeBytes)
{
  static std::list<int32> uniformBlocksBound;
  auto iter = std::find(uniformBlocksBound.begin(), uniformBlocksBound.end(), location);
  if (iter != uniformBlocksBound.end())
  {
    return;
  }

  glUniformBlockBinding(_programId, location, bindingPoint);
  glBindBufferRange(GL_UNIFORM_BUFFER, bindingPoint, ubo, 0, sizeBytes);
  uniformBlocksBound.push_back(location);
}

int32 Shader::GetAttributeLocation(const std::string& name)
{
  Bind();
  int32 location = glGetAttribLocation(_programId, name.c_str());
  if (location < 0)
  {
    std::stringstream message;
    message << "Could not find attribute '" << name << "' for shader '" << _fileName << "' bound to program " << _programId;
    //throw std::runtime_error(message.str());
  }
  return location;
}

int32 Shader::GetUniformBlockIndex(const std::string& name)
{
  Bind();
  int32 location = glGetUniformBlockIndex(_programId, name.c_str());
  if (location < 0)
  {
    // TODO: error handling
  }
  return location;
}

void Shader::Bind()
{
  if (!IsBound())
  {
    glUseProgram(_programId);
    s_activeShader = _programId;
  }
}

void Shader::Unbind()
{
  if (IsBound())
  {
    glUseProgram(0);
    s_activeShader = 0;
  }
}

bool Shader::IsBound() const
{
  return s_activeShader == _programId;
}

void Shader::LoadFromFile()
{
  std::fstream fstream;
  fstream.open(_fileName, std::fstream::in);
  if (!fstream.is_open())
  {
    throw std::runtime_error("Failed to open file '" + _fileName + "'");
  }

  std::string source = ReadSource(fstream);
  fstream.close();

  size_t newLinePos = source.find('\n');
  std::string shaderVersion = source.substr(0, newLinePos + 1);
  source = source.substr(newLinePos + 1);

  uint32 vertexShaderId = AttachSource(ShaderType::Vertex, shaderVersion + "#define VERTEX_SHADER\n" + source);
  uint32 fragmentShaderId = AttachSource(ShaderType::Fragment, shaderVersion + "#define FRAGMENT_SHADER\n" + source);

  AttachShaders(vertexShaderId, fragmentShaderId);
  Link();
}

std::string Shader::ReadSource(std::fstream& fstream)
{
  fstream.seekg(0, fstream.end);
  std::streampos fileLength = fstream.tellg();
  fstream.seekg(0, fstream.beg);

  std::vector<byte> buffer((int32)fileLength);
  fstream.read(&buffer[0], fileLength);
  
  return std::string(buffer.begin(), buffer.end());
}

uint32 Shader::AttachSource(ShaderType shaderType, const std::string& shaderSource)
{
  uint32 shaderId = glCreateShader(FromShaderType(shaderType));
  if (shaderId == 0)
  {
    throw std::runtime_error("OpenGL was unable to create a shader object for shader '" + _fileName + "'");
  }

  const byte* ptr = shaderSource.c_str();
  glShaderSource(shaderId, 1, &ptr, nullptr);

  Compile(shaderId);
  return shaderId;
}

void Shader::Compile(uint32 shaderId)
{
  int32 compiled = -1;
  glCompileShader(shaderId);
  glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compiled);

  if (compiled == GL_FALSE)
  {
    int32 logLength = -1;
    glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &logLength);

    std::stringstream message;
    message << "Failed to compile shader bound to ID " << shaderId;

    if (logLength > 0)
    {
      std::vector<byte> buffer(logLength);
      glGetShaderInfoLog(shaderId, logLength, nullptr, &buffer[0]);
      std::string log(buffer.begin(), buffer.end());
      message << ": " << log;
    }
    throw std::runtime_error(message.str());
  }
}

void Shader::AttachShaders(uint32 vertexShaderId, uint32 fragmentShaderId)
{
  glAttachShader(_programId, vertexShaderId);
  glDeleteShader(vertexShaderId);
  glAttachShader(_programId, fragmentShaderId);
  glDeleteShader(fragmentShaderId);
}

void Shader::Link()
{
  int32 linked = -1;
  glLinkProgram(_programId);
  glGetProgramiv(_programId, GL_LINK_STATUS, &linked);

  if (linked == GL_FALSE)
  {
    int32 logLength = -1;
    glGetProgramiv(_programId, GL_INFO_LOG_LENGTH, &logLength);

    std::stringstream message;
    message << "Failed to link shaders from '" << _fileName << "' bound to ID " + _programId;

    if (logLength > 0)
    {
      std::vector<char> buffer(logLength);
      glGetProgramInfoLog(_programId, logLength, nullptr, &buffer[0]);
      std::string log(buffer.begin(), buffer.end());
      message << ": " << log;
    }
    throw std::runtime_error(message.str());
  }
}

void Shader::BuildUniformDeclaration()
{
  GLint activeUniformCount = -1;
  glGetProgramiv(_programId, GL_ACTIVE_UNIFORMS, &activeUniformCount);
  _uniforms.reserve(activeUniformCount);

  for (GLint i = 0; i < activeUniformCount; i++)
  {
    GLint size;
    GLenum type;
    GLchar name[255];
    glGetActiveUniform(_programId, i, 255, nullptr, &size, &type, name);
    _uniforms.emplace(std::make_pair(name, ShaderUniform(i, size, ToShaderDataType(type), name)));
  }
}

int32 Shader::GetUniformLocation(const std::string& name)
{
  auto iter = _uniforms.find(name);
  if (iter == _uniforms.end())
  {
    std::stringstream message;
    message << "Could not find uniform '" << name << "' for shader '" << _fileName << "'";
    throw std::runtime_error(message.str());
  }
  return iter->second.Index;
}
}