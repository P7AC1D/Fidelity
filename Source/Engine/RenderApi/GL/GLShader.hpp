#pragma once
#include <unordered_map>
#include <unordered_set>
#include "../Shader.hpp"

class GLShader : public Shader
{
	friend class GLRenderDevice;

public:
	~GLShader();

	uint32 GetId() const { return _id; }

	void Compile() override;

	bool HasUniform(const std::string &name) const;

	void BindUniformBlock(const std::string &name, uint32 bindingPoint);
	void BindTextureUnit(const std::string &name, uint32 textureUnit);

private:
	GLShader(const ShaderDesc &desc);

	void AttachHeaderFiles();
	std::string GetShaderLog();

	uint32 GetUniformLocation(const std::string &name);

	void BuildUniformDefinitions();
	void BuildUniformBlockDefinitions();

private:
	enum class UniformType
	{
		Sampler2D,
		Sampler2DArray,
		SamplerCube,
		UniformBlock
	};

	struct Uniform
	{
		uint32 Location;
		std::string Name;
		UniformType Type;
	};

	uint32 _id;
	std::unordered_map<std::string, Uniform> _uniforms;
	std::unordered_map<uint32, uint32> _assignedBindingPoints;
	std::unordered_map<uint32, uint32> _assignedTextureSlots;
};
