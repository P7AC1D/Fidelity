#include "GLShader.hpp"

#include <sstream>
#include "../../Utility/Assert.hpp"
#include "../../Utility/Hash.hpp"
#include "GL.hpp"

GLenum GetShaderType(ShaderType shaderType)
{
  switch (shaderType)
  {
    case ShaderType::Vertex: return GL_VERTEX_SHADER;
    case ShaderType::Pixel: return GL_FRAGMENT_SHADER;
    case ShaderType::Domain: return GL_TESS_CONTROL_SHADER;
    case ShaderType::Hull: return GL_TESS_EVALUATION_SHADER;
    case ShaderType::Geometry: return GL_GEOMETRY_SHADER;
  }
  throw std::runtime_error("Unsupported ShaderType");
}

GLShader::~GLShader()
{
  if (_id != 0)
  {
    GLCall(glDeleteProgram(_id));
    _id = 0;
  }
}

void GLShader::Compile()
{
  if (!IsCompiled())
  {
    return;
  }
  
  const byte* ptr = _desc.Source.c_str();
  _id = glCreateShaderProgramv(GetShaderType(_desc.ShaderType), 1, &ptr);
  Assert::ThrowIfTrue(_id == 0, "Unable to generate shader object");
  
  int32 logLength = -1;
  GLCall(glGetProgramiv(_id, GL_INFO_LOG_LENGTH, &logLength));
  
  std::stringstream message;
  message << "Shader compile log";
  
  if (logLength > 0)
  {
    std::vector<byte> buffer(logLength);
    GLCall(glGetProgramInfoLog(_id, logLength, 0, &buffer[0]));
    std::string log(buffer.begin(), buffer.end());
    message << ": " << log;
  }
  
  _shaderLog = message.str();
  _isCompiled = true;
}

void GLShader::BindUniformBlock(const std::string& name, uint32 bindingPoint)
{
  auto blockIndex = GetUniformBlockIndex(name);
  auto bindingPointIter = _uniformBindingPoints.find(blockIndex);
  if (bindingPointIter != _uniformBindingPoints.end() && bindingPointIter->second == bindingPoint)
  {
    return;
  }
  
  GLCall(glUniformBlockBinding(_id, blockIndex, bindingPoint));
  _uniformBindingPoints[blockIndex] = bindingPoint;
}

GLShader::GLShader(const ShaderDesc& desc): Shader(desc), _id(0)
{
  Assert::ThrowIfTrue(desc.ShaderLang != ShaderLang::Glsl, "Shaders must be written in GLSL when using OpenGL backend");
  Assert::ThrowIfTrue(desc.Source.empty(), "Shader source is empty");
  Assert::ThrowIfTrue(desc.EntryPoint.empty(), "Shader entry point not defined");
  Assert::ThrowIfFalse(desc.EntryPoint == "main", "GLSL shaders must have a 'main' entry point");
}

uint32 GLShader::GetUniformBlockIndex(const std::string& name)
{
  auto blockIndexIter = _uniformBlockIndices.find(name);
  if (blockIndexIter == _uniformBlockIndices.end())
  {
    auto blockIndex = glGetUniformBlockIndex(_id, name.c_str());
    Assert::ThrowIfFalse(blockIndex == 0, "Could not find a valid uniform block index with name " + name);
    _uniformBlockIndices.insert(std::pair(name, blockIndex));
    return blockIndex;
  }
  return blockIndexIter->second;
}
