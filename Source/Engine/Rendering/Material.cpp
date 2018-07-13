#include "Material.hpp"

#include "../RenderApi/Texture.hpp"

void Material::SetAmbientColour(const Colour& ambientColour)
{
	_ambientColour = ambientColour;
}

void Material::SetDiffuseColour(const Colour& diffuseColour)
{
	_diffuseColour = diffuseColour;
}

void Material::SetSpecularColour(const Colour& specularColour)
{
	_specularColour = specularColour;
}

void Material::SetSpecularExponent(float32 specularExponent)
{
	_specularExponent = specularExponent;
}

void Material::SetDiffuseTexture(const std::shared_ptr<Texture>& diffuseTexture)
{
	_diffuseTexture = diffuseTexture;
}

void Material::SetNormalTexture(const std::shared_ptr<Texture>& normalTexture)
{
	_normalTexture = normalTexture;
}

void Material::SetSpecularTexture(const std::shared_ptr<Texture>& specularTexture)
{
	_specularTexture = specularTexture;
}

void Material::SetDepthTexture(const std::shared_ptr<Texture>& depthTexture)
{
	_depthTexture = depthTexture;
}

uint32 Material::GetId() const
{
	return _id;
}

Colour Material::GetAmbientColour() const
{
	return _ambientColour;
}

Colour Material::GetDiffuseColour() const
{
	return _diffuseColour;
}

Colour Material::GetSpecularColour() const
{
	return _specularColour;
}

float32 Material::GetSpecularExponent() const
{
	return _specularExponent;
}

std::shared_ptr<Texture> Material::GetDiffuseTexture() const
{
	return _diffuseTexture;
}

std::shared_ptr<Texture> Material::GetNormalTexture() const
{
	return _normalTexture;
}

std::shared_ptr<Texture> Material::GetSpecularTexture() const
{
	return _specularTexture;
}

std::shared_ptr<Texture> Material::GetDepthTexture() const
{
	return _depthTexture;
}

bool Material::HasDiffuseTexture() const
{
	return _diffuseTexture != nullptr;
}

bool Material::HasNormalTexture() const
{
	return _normalTexture != nullptr;
}

bool Material::HasSpecularTexture() const
{
	return _specularTexture != nullptr;
}

bool Material::HasDepthTexture() const
{
	return _depthTexture != nullptr;
}

Material::Material() : _id(0), _specularExponent(1.0f)
{}