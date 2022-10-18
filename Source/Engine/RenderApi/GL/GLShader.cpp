#include "GLShader.hpp"

#include "../../Utility/Assert.hpp"
#include "../../Utility/Hash.hpp"
#include "../../Utility/String.hpp"
#include "GL.hpp"

GLenum getShaderType(ShaderType shaderType)
{
	switch (shaderType)
	{
	case ShaderType::Vertex:
		return GL_VERTEX_SHADER;
	case ShaderType::Fragment:
		return GL_FRAGMENT_SHADER;
	case ShaderType::Domain:
		return GL_TESS_CONTROL_SHADER;
	case ShaderType::Hull:
		return GL_TESS_EVALUATION_SHADER;
	case ShaderType::Geometry:
		return GL_GEOMETRY_SHADER;
	}
	throw std::runtime_error("Unsupported ShaderType");
}

GLShader::~GLShader()
{
	if (_id != 0)
	{
		glCall(glDeleteProgram(_id));
		_id = 0;
	}
}

void GLShader::compile()
{
	if (isCompiled())
	{
		return;
	}

	attachHeaderFiles();
	const byte *ptr = _desc.Source.c_str();
	glCall2(glCreateShaderProgramv(getShaderType(_desc.ShaderType), 1, &ptr), _id);

	ASSERT_FALSE(_id == 0, "Unable to generate shader object");
	glCall(glUseProgram(0));

	GLint linkStatus = -1;
	glCall(glGetProgramiv(_id, GL_LINK_STATUS, &linkStatus));

	if (linkStatus == GL_FALSE)
	{
		std::string errorMessage = "Unable to compile shader:\n" + getShaderLog();
		throw std::runtime_error(errorMessage);
	}
	_isCompiled = true;

	buildUniformDefinitions();
	buildUniformBlockDefinitions();
}

bool GLShader::hasUniform(const std::string &name) const
{
	return _uniforms.find(name) != _uniforms.end();
}

void GLShader::bindUniformBlock(const std::string &name, uint32 bindingPoint)
{
	auto iter = _assignedBindingPoints.find(bindingPoint);
	auto uniformLocation = getUniformLocation(name);
	if (iter == _assignedBindingPoints.end() || iter->second != uniformLocation)
	{
		glCall(glUniformBlockBinding(_id, uniformLocation, bindingPoint));
		_assignedBindingPoints[bindingPoint] = uniformLocation;
	}
}

void GLShader::bindTextureUnit(const std::string &name, uint32 textureUnit)
{
	auto iter = _assignedTextureSlots.find(textureUnit);
	auto uniformLocation = getUniformLocation(name);
	if (iter == _assignedTextureSlots.end() || iter->second != uniformLocation)
	{
		glCall(glProgramUniform1i(_id, uniformLocation, textureUnit));
		_assignedTextureSlots[textureUnit] = uniformLocation;
	}
}

GLShader::GLShader(const ShaderDesc &desc) : Shader(desc), _id(0)
{
	ASSERT_FALSE(desc.ShaderLang != ShaderLang::Glsl, "Shaders must be written in GLSL when using OpenGL backend");
	ASSERT_FALSE(desc.Source.empty(), "Shader source is empty");
	ASSERT_FALSE(desc.EntryPoint.empty(), "Shader entry point not defined");
	ASSERT_TRUE(desc.EntryPoint == "main", "GLSL shaders must have a 'main' entry point");
}

void GLShader::attachHeaderFiles()
{
	for (auto iterPos = 0; iterPos != std::string::npos;)
	{
		iterPos = _desc.Source.find("#include", iterPos);
		if (iterPos != std::string::npos)
		{
			auto newLinePos = _desc.Source.find("\n", iterPos);
			auto line = _desc.Source.substr(iterPos, newLinePos - iterPos);
			auto splitLine = String::split(line, '\"');
			ASSERT_TRUE(splitLine.size() == 3, "#include syntax error");

			auto headerSource = String::foadFromFile("./Shaders/" + splitLine[1]);

			_desc.Source.erase(iterPos, newLinePos - iterPos);
			_desc.Source.insert(iterPos, headerSource.c_str());
		}
	}
}

std::string GLShader::getShaderLog()
{
	int32 logLength = -1;
	glCall(glGetProgramiv(_id, GL_INFO_LOG_LENGTH, &logLength));

	if (logLength > 0)
	{
		std::vector<byte> buffer(logLength);
		glCall(glGetProgramInfoLog(_id, logLength, 0, &buffer[0]));
		return std::string(buffer.begin(), buffer.end());
	}
	return std::string();
}

uint32 GLShader::getUniformLocation(const std::string &name)
{
	auto uniformIter = _uniforms.find(name);
	if (uniformIter == _uniforms.end())
	{
		return -1;
	}
	return uniformIter->second.Location;
}

void GLShader::buildUniformDefinitions()
{
	GLint activeUniformCount = -1;
	glCall(glGetProgramiv(_id, GL_ACTIVE_UNIFORMS, &activeUniformCount));
	for (GLint i = 0; i < activeUniformCount; i++)
	{
		GLint size;
		GLenum type;
		GLchar name[255];
		glCall(glGetActiveUniform(_id, i, 255, NULL, &size, &type, name));

		GLint location = -1;
		glCall2(glGetUniformLocation(_id, name), location);

		if (type == GL_SAMPLER_2D)
		{
			_uniforms.emplace(name, Uniform{static_cast<uint32>(location), name, UniformType::Sampler2D});
		}
		else if (type == GL_SAMPLER_2D_ARRAY)
		{
			_uniforms.emplace(name, Uniform{static_cast<uint32>(location), name, UniformType::Sampler2DArray});
		}
		else if (type == GL_SAMPLER_CUBE)
		{
			_uniforms.emplace(name, Uniform{static_cast<uint32>(location), name, UniformType::SamplerCube});
		}
	}
}

void GLShader::buildUniformBlockDefinitions()
{
	GLint activeUniformBlockCount = -1;
	glCall(glGetProgramiv(_id, GL_ACTIVE_UNIFORM_BLOCKS, &activeUniformBlockCount));
	for (GLint i = 0; i < activeUniformBlockCount; i++)
	{
		GLchar uniformBlockName[255];
		glCall(glGetActiveUniformBlockName(_id, i, 255, NULL, uniformBlockName));

		GLuint blockIndex = -1;
		glCall2(glGetUniformBlockIndex(_id, uniformBlockName), blockIndex);
		_uniforms.emplace(uniformBlockName, Uniform{blockIndex, uniformBlockName, UniformType::UniformBlock});
	}
}
