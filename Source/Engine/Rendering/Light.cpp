#include "Light.h"

#include "../UI/ImGui/imgui.h"
#include "../Core/GameObject.h"

Light::Light() : Component(std::string()),
								 _colour(Colour::White),
								 _radius(10.0f),
								 _lightType(LightType::Point),
								 _modified(true)
{
}

void Light::drawInspector()
{
	ImGui::Separator();
	ImGui::Text("LightNode");

	float32 rawCol[]{_colour[0], _colour[1], _colour[2]};
	ImGui::ColorEdit3("Colour", rawCol);
	setColour(Colour(rawCol[0] * 255, rawCol[1] * 255, rawCol[2] * 255));

	ImGui::DragFloat("Radius", &_radius, 0.1f, 0.0f, 1000.0f);
}

Light &Light::setColour(const Colour &colour)
{
	_colour = colour;
	_modified = true;
	return *this;
}

Light &Light::setRadius(float32 radius)
{
	_radius = radius;
	_modified = true;
	return *this;
}

Light &Light::setLightType(LightType lightType)
{
	_lightType = lightType;
	_modified = true;
	return *this;
}

void Light::onUpdate(float32 dt)
{
	if (_modified)
	{
		Matrix4 translation = Matrix4::Translation(_position);
		Matrix4 scale = Matrix4::Scaling(Vector3(_radius));
		Matrix4 rotation = Matrix4::Rotation(_rotation);
		_matrix = translation * scale * rotation;
	}
}

void Light::onNotify(const GameObject &gameObject)
{
	Transform transform = gameObject.getTransform();
	_position = transform.getPosition();
	_rotation = transform.getRotation();
	_modified = true;
}