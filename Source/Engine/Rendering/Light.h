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
	Light &setIntensity(float32 intensity);
	
	// Shadow system support
	Light &setCastsShadows(bool castsShadows);
	Light &setShadowResolution(uint32 resolution);
	Light &setShadowNearPlane(float32 nearPlane);
	Light &setShadowFarPlane(float32 farPlane);

	Matrix4 getMatrix() const { return _matrix; }
	Vector3 getPosition() const { return _position; }

	Colour getColour() const { return _colour; }
	float32 getRadius() const { return _radius; }
	LightType getLightType() const { return _lightType; }
	Vector3 getDirection() const { return _direction; }
	float32 getIntensity() const { return _intensity; }
	
	// Shadow getters
	bool getCastsShadows() const { return _castsShadows; }
	uint32 getShadowResolution() const { return _shadowResolution; }
	float32 getShadowNearPlane() const { return _shadowNearPlane; }
	float32 getShadowFarPlane() const { return _shadowFarPlane; }

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
	float32 _intensity;

	// Shadow properties
	bool _castsShadows;
	uint32 _shadowResolution;
	float32 _shadowNearPlane;
	float32 _shadowFarPlane;

	bool _modified;
};
