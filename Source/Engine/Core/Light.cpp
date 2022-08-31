#include "Light.h"

#include "../UI/ImGui/imgui.h"
#include "../Maths/Degree.hpp"

Light::Light() : _colour(Colour::White),
								 _radius(10.0f),
								 _lightType(LightType::Point),
								 _modified(true)
{
}

void Light::drawInspector()
{
	ImGui::Separator();
	ImGui::Text("LightNode");

	float32 pos[]{_position.X, _position.Y, _position.Z};
	ImGui::DragFloat3("Position", pos, 0.1f);
	setPosition(Vector3(pos[0], pos[1], pos[2]));

	Vector3 euler = _rotation.ToEuler();
	float32 angles[3] = {euler.X, euler.Y, euler.Z};
	ImGui::DragFloat3("Orientation", angles, 1.0f);
	setRotation(Quaternion(Degree(angles[0]), Degree(angles[1]), Degree(angles[2])));

	float32 rawCol[]{_colour[0], _colour[1], _colour[2]};
	ImGui::ColorEdit3("Colour", rawCol);
	setColour(Colour(rawCol[0] * 255, rawCol[1] * 255, rawCol[2] * 255));

	ImGui::DragFloat("Radius", &_radius, 0.1f, 0.0f, 1000.0f);
}

void Light::update(float32 dt)
{
	if (_modified)
	{
		Matrix4 translation = Matrix4::Translation(_position);
		Matrix4 scale = Matrix4::Scaling(Vector3(_radius));
		Matrix4 rotation = Matrix4::Rotation(_rotation);
		_transform = translation * scale * rotation;
	}
}

Light &Light::setRotation(const Quaternion &rotation)
{
	_rotation = rotation;
	_modified = true;
	return *this;
}

Light &Light::setPosition(const Vector3 &position)
{
	_position = position;
	_modified = true;
	return *this;
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