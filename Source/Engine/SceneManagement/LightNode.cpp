#include "LightNode.hpp"

#include "../Rendering/Renderer.h"
#include "../UI/ImGui/imgui.h"

LightNode::LightNode() :
	_colour(Colour::White),
	_intensity(1.0f),
	_lightType(LightType::Point)
{
}

void LightNode::OnDraw(std::shared_ptr<Renderer> renderer)
{
	renderer->SubmitLight(std::static_pointer_cast<LightNode>(shared_from_this()));
}

void LightNode::OnDrawInspector()
{
	ImGui::Separator();
	ImGui::Text("LightNode");
	
	float32 rawCol[]{ _colour[0], _colour[1], _colour[2] };
	ImGui::ColorEdit3("Colour", rawCol);
	SetColour(Colour(rawCol[0] * 255, rawCol[1] * 255, rawCol[2] * 255));

	ImGui::DragFloat("Intensity", &_intensity, 0.01f, 0.0f, 1.0f);
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

SceneNodeType LightNode::GetNodeType() const
{
	return SceneNodeType::Light;
}