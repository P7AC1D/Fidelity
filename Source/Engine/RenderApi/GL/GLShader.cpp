#include "GLShader.hpp"

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
  if (IsCompiled())
  {
    return;
  }
  
  const byte* ptr = _desc.Source.c_str();
	GLCall2(glCreateShaderProgramv(GetShaderType(_desc.ShaderType), 1, &ptr), _id);
  Assert::ThrowIfTrue(_id == 0, "Unable to generate shader object");
  
  int32 logLength = -1;
  GLCall(glGetProgramiv(_id, GL_INFO_LOG_LENGTH, &logLength));

	GLint linkStatus = -1;
	GLCall(glGetProgramiv(_id, GL_LINK_STATUS, &linkStatus));
  
  std::string logMessages;
  if (logLength > 0)
  {
    std::vector<byte> buffer(logLength);
    GLCall(glGetProgramInfoLog(_id, logLength, 0, &buffer[0]));
		logMessages = std::string(buffer.begin(), buffer.end());
  }

	GLCall(glUseProgram(0));
	Assert::ThrowIfFalse(linkStatus == GL_TRUE, "Unable to compile shader:\n" + logMessages);
  _isCompiled = true;

	BuildUniformDefinitions();
	BuildUniformBlockDefinitions();
}

bool GLShader::HasUniform(const std::string& name) const
{
	Uniform expected;
	expected.Name = name;
	return _uniforms.find(expected) != _uniforms.end();
}

void GLShader::BindUniformBlock(const std::string& name, uint32 bindingPoint)
{
	GLCall(glUniformBlockBinding(_id, GetUniformLocation(name), bindingPoint));
}

void GLShader::BindTextureUnit(const std::string& name, uint32 textureUnit)
{
	GLCall(glProgramUniform1i(_id, GetUniformLocation(name), textureUnit));
}

GLShader::GLShader(const ShaderDesc& desc): Shader(desc), _id(0)
{
  Assert::ThrowIfTrue(desc.ShaderLang != ShaderLang::Glsl, "Shaders must be written in GLSL when using OpenGL backend");
  Assert::ThrowIfTrue(desc.Source.empty(), "Shader source is empty");
  Assert::ThrowIfTrue(desc.EntryPoint.empty(), "Shader entry point not defined");
  Assert::ThrowIfFalse(desc.EntryPoint == "main", "GLSL shaders must have a 'main' entry point");
}

uint32 GLShader::GetUniformLocation(const std::string& name)
{
	Uniform expected;
	expected.Name = name;
	auto uniformIter = _uniforms.find(expected);
	if (uniformIter == _uniforms.end())
	{
		return -1;
	}
	return uniformIter->Location;
}

void GLShader::BuildUniformDefinitions()
{
	GLint activeUniformCount = -1;
	GLCall(glGetProgramiv(_id, GL_ACTIVE_UNIFORMS, &activeUniformCount));
	for (GLint i = 0; i < activeUniformCount; i++)
	{
		GLint size;
		GLenum type;
		GLchar name[255];
		GLCall(glGetActiveUniform(_id, i, 255, NULL, &size, &type, name));

		GLint location = -1;
		GLCall2(glGetUniformLocation(_id, name), location);

		if (type == GL_SAMPLER_2D)
		{
			_uniforms.insert(Uniform{ static_cast<uint32>(location), name, UniformType::Sampler2D });
		}
		else if (type == GL_SAMPLER_CUBE)
		{
			_uniforms.insert(Uniform{ static_cast<uint32>(location), name, UniformType::SamplerCube });
		}
	}
}

void GLShader::BuildUniformBlockDefinitions()
{
	GLint activeUniformBlockCount = -1;
	GLCall(glGetProgramiv(_id, GL_ACTIVE_UNIFORM_BLOCKS, &activeUniformBlockCount));
	for (GLint i = 0; i < activeUniformBlockCount; i++)
	{
		GLchar uniformBlockName[255];
		GLCall(glGetActiveUniformBlockName(_id, i, 255, NULL, uniformBlockName));

		GLuint blockIndex = -1;
		GLCall2(glGetUniformBlockIndex(_id, uniformBlockName), blockIndex);
		_uniforms.insert(Uniform{ blockIndex, uniformBlockName, UniformType::UniformBlock });
	}
}

std::size_t GLShader::Hash::operator()(const Uniform& uniform) const
{
	std::size_t seed = 0;
	::Hash::Combine(seed, uniform.Name);
	return seed;
}

bool GLShader::Equal::operator()(const Uniform& a, const Uniform& b) const
{
	return a.Name == b.Name;
}