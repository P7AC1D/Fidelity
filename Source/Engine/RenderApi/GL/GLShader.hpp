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

	void compile() override;

	bool hasUniform(const std::string &name) const;

	void bindUniformBlock(const std::string &name, uint32 bindingPoint);
	void bindTextureUnit(const std::string &name, uint32 textureUnit);

private:
	GLShader(const ShaderDesc &desc);

	void attachHeaderFiles();
	std::string getShaderLog();

	uint32 getUniformLocation(const std::string &name);

	void buildUniformDefinitions();
	void buildUniformBlockDefinitions();

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
