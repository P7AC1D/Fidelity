#include "Shader.h"

#include <algorithm>
#include <exception>
#include <fstream>
#include <list>
#include <sstream>

namespace Rendering
{
static uint32 s_activeShader = 0;

Shader::Shader(const std::string& fileName) :
  m_shaderFileName(fileName)
{
  m_programId = glCreateProgram();
  if (m_programId == 0)
  {
    std::stringstream message;
    message << "OpenGL was unable to create a shader program object for shader " << m_shaderFileName;
    throw std::runtime_error(message.str());
  }
  LoadFromFile();
}

Shader::~Shader()
{
  glDeleteProgram(m_programId);
  m_programId = 0;
}

void Shader::SetUniformInt(int32 location, int32 value)
{
  Bind();
  glUniform1i(location, value);
}

void Shader::SetUniformFloat(int32 location, float32 value)
{
  Bind();
  glUniform1f(location, value);
}

void Shader::SetUniformVec3(int32 location, const Vector3& value)
{
  Bind();
  glUniform3f(location, value[0], value[1], value[2]);
}

void Shader::SetUniformVec4(int32 location, const Vector4& value)
{
  Bind();
  glUniform4f(location, value[0], value[1], value[2], value[3]);
}

void Shader::SetUniformMat3(int32 location, const Matrix3& value)
{
  Bind();
  glUniformMatrix3fv(location, 1, GL_FALSE, &value[0][0]);
}

void Shader::SetUniformMat4(int32 location, const Matrix4& value)
{
  Bind();
  glUniformMatrix4fv(location, 1, GL_FALSE, &value[0][0]);
}

void Shader::SetUniformVec3Array(int32 location, const std::vector<Vector3>& values)
{
  Bind();
  glUniform3fv(location, static_cast<int32>(values.size()), values[0].Ptr());
}

void Shader::SetUniformVec4Array(int32 location, const std::vector<Vector4>& values)
{
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

  glUniformBlockBinding(m_programId, location, bindingPoint);
  glBindBufferRange(GL_UNIFORM_BUFFER, bindingPoint, ubo, 0, sizeBytes);
  uniformBlocksBound.push_back(location);
}

int32 Shader::GetUniformLocation(const std::string& name)
{
  Bind();
  int32 location = glGetUniformLocation(m_programId, name.c_str());
  if (location < 0)
  {
    std::stringstream message;
    message << "Could not find uniform '" << name << "' for shader '" << m_shaderFileName << "' bound to program " << m_programId;
    //throw std::runtime_error(message.str());
  }
  return location;
}

int32 Shader::GetAttributeLocation(const std::string& name)
{
  Bind();
  int32 location = glGetAttribLocation(m_programId, name.c_str());
  if (location < 0)
  {
    std::stringstream message;
    message << "Could not find attribute '" << name << "' for shader '" << m_shaderFileName << "' bound to program " << m_programId;
    //throw std::runtime_error(message.str());
  }
  return location;
}

int32 Shader::GetUniformBlockIndex(const std::string& name)
{
  Bind();
  int32 location = glGetUniformBlockIndex(m_programId, name.c_str());
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
    glUseProgram(m_programId);
    s_activeShader = m_programId;
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
  return s_activeShader == m_programId;
}

void Shader::LoadFromFile()
{
  std::fstream fstream;
  fstream.open(m_shaderFileName, std::fstream::in);
  if (!fstream.is_open())
  {
    throw std::runtime_error("Failed to open file '" + m_shaderFileName + "'");
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
  uint32 shaderId = glCreateShader(static_cast<uint32>(shaderType));
  if (shaderId == 0)
  {
    throw std::runtime_error("OpenGL was unable to create a shader object for shader '" + m_shaderFileName + "'");
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
  glAttachShader(m_programId, vertexShaderId);
  glDeleteShader(vertexShaderId);
  glAttachShader(m_programId, fragmentShaderId);
  glDeleteShader(fragmentShaderId);
}

void Shader::Link()
{
  int32 linked = -1;
  glLinkProgram(m_programId);
  glGetProgramiv(m_programId, GL_LINK_STATUS, &linked);

  if (linked == GL_FALSE)
  {
    int32 logLength = -1;
    glGetProgramiv(m_programId, GL_INFO_LOG_LENGTH, &logLength);

    std::stringstream message;
    message << "Failed to link shaders from '" << m_shaderFileName << "' bound to ID " + m_programId;

    if (logLength > 0)
    {
      std::vector<char> buffer(logLength);
      glGetProgramInfoLog(m_programId, logLength, nullptr, &buffer[0]);
      std::string log(buffer.begin(), buffer.end());
      message << ": " << log;
    }
    throw std::runtime_error(message.str());
  }
}
}