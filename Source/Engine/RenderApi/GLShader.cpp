#include "GLShader.hpp"

#include <sstream>
#include "../Utility/Assert.hpp"
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
}

void GLShader::Compile()
{
  if (!IsCompiled())
  {
    return;
  }
  
  _id = glCreateShader(GetShaderType(_desc.ShaderType));
  if (_id == 0)
  {
    throw std::runtime_error("Unable to generate shader object");
  }
  
  const byte* ptr = _desc.Source.c_str();
  GLCall(glShaderSource(_id, 1, &ptr, 0));
  GLCall(glCompileShader(_id));
  
  int32 logLength = -1;
  GLCall(glGetShaderiv(_id, GL_INFO_LOG_LENGTH, &logLength));
  
  std::stringstream message;
  message << "Shader compile log";
  
  if (logLength > 0)
  {
    std::vector<byte> buffer(logLength);
    GLCall(glGetShaderInfoLog(_id, logLength, 0, &buffer[0]));
    std::string log(buffer.begin(), buffer.end());
    message << ": " << log;
  }
  
  _shaderLog = message.str();
  _isCompiled = true;
}

GLShader::GLShader(const ShaderDesc& desc): _id(0)
{
  Assert::ThrowIfTrue(desc.ShaderLang != ShaderLang::Glsl, "Shaders must be written in GLSL when using OpenGL backend");
  Assert::ThrowIfTrue(desc.Source.empty(), "Shader source is empty");
  Assert::ThrowIfTrue(desc.EntryPoint.empty(), "Shader entry point not defined");
  
  _desc = desc;
}
