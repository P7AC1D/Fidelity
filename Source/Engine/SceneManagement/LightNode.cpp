#include "LightNode.hpp"

#include "../Rendering/Renderer.h"

LightNode::LightNode() :
	_colour(Colour::White),
	_intensity(1.0f),
	_lightType(LightType::Point)
{
}

void LightNode::OnDraw(std::shared_ptr<Renderer> renderer)
{
	
}

Colour LightNode::GetColour() const
{
	return _colour;
}

void LightNode::SetColour(const Colour& colour)
{
	_colour = colour;
}

float32 LightNode::GetIntensity() const
{
	return _intensity;
}

void LightNode::SetIntensity(const float32 intensity)
{
	_intensity = intensity;
}

LightType LightNode::GetLightType() const
{
	return _lightType;
}

void LightNode::SetLightType(const LightType lightType)
{
	_lightType = lightType;
}
