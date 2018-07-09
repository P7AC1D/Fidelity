#pragma once
#include <memory>
#include "../Maths/Colour.hpp"

class Texture;

class Material
{
public:
	Material();

	void SetAmbientColour(const Colour& ambientColour);
	void SetDiffuseColour(const Colour& diffuseColour);
	void SetSpecularColour(const Colour& specularColour);
	void SetSpecularExponent(float32 specularExponent);

	void SetDiffuseTexture(const std::shared_ptr<Texture>& diffuseTexture);
	void SetNormalTexture(const std::shared_ptr<Texture>& normalTexture);
	void SetSpecularTexture(const std::shared_ptr<Texture>& specularTexture);
	void SetDepthTexture(const std::shared_ptr<Texture>& depthTexture);

	Colour GetAmbientColour() const;
	Colour GetDiffuseColour() const;
	Colour GetSpecularColour() const;
	float32 GetSpecularExponent() const;

	std::shared_ptr<Texture> GetDiffuseTexture() const;
	std::shared_ptr<Texture> GetNormalTexture() const;
	std::shared_ptr<Texture> GetSpecularTexture() const;
	std::shared_ptr<Texture> GetDepthTexture() const;

	bool HasDiffuseTexture() const;
	bool HasNormalTexture() const;
	bool HasSpecularTexture() const;
	bool HasDepthTexture() const;

private:
	Colour _ambientColour;
	Colour _diffuseColour;
	Colour _specularColour;
	float32 _specularExponent;

	std::shared_ptr<Texture> _diffuseTexture;
	std::shared_ptr<Texture> _normalTexture;
	std::shared_ptr<Texture> _specularTexture;
	std::shared_ptr<Texture> _depthTexture;
};