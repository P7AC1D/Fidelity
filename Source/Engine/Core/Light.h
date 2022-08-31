#pragma once
#include "Maths.h"

enum class LightType
{
	Point,
	Directional,
	Spot,
	Area
};

class Light
{
public:
	Light();

	void drawInspector();

	void update(float32 dt);

	Light &setPosition(const Vector3 &position);
	Light &setRotation(const Quaternion &rotation);
	Light &setColour(const Colour &colour);
	Light &setRadius(float32 radius);
	Light &setLightType(LightType lightType);

	Matrix4 getTransform() const { return _transform; }
	Vector3 getPosition() const { return _position; }
	Quaternion getRotation() const { return _rotation; }
	Colour getColour() const { return _colour; }
	float32 getRadius() const { return _radius; }
	LightType getLightType() const { return _lightType; }

private:
	Colour _colour;
	float32 _radius;
	LightType _lightType;
	Vector3 _position;
	Quaternion _rotation;
	Matrix4 _transform;

	bool _modified;
};
