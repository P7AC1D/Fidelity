#pragma once
#include "../Core/Maths.h"
#include "../Core/Component.h"

enum class LightType
{
	Point,
	Directional,
	Spot,
	Area
};

class Light : public Component
{
public:
	Light();

	void drawInspector() override;

	Light &setColour(const Colour &colour);
	Light &setRadius(float32 radius);
	Light &setLightType(LightType lightType);
	Light &setDirection(const Vector3 &direction);

	Matrix4 getMatrix() const { return _matrix; }
	Vector3 getPosition() const { return _position; }

	Colour getColour() const { return _colour; }
	float32 getRadius() const { return _radius; }
	LightType getLightType() const { return _lightType; }
	Vector3 getDirection() const { return _direction; }

private:
	void onUpdate(float32 dt) override;
	void onNotify(const GameObject &gameObject) override;

	Colour _colour;
	float32 _radius;
	LightType _lightType;
	Vector3 _position;
	Quaternion _rotation;
	Matrix4 _matrix;
	Vector3 _direction;

	bool _modified;
};
