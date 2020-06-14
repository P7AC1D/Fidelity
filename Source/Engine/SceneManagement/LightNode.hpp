#pragma once
#include "SceneNode.hpp"
#include "../Maths/Colour.hpp"

enum class LightType
{
	Point,
	Directional,
	Spot,
	Area
};

class LightNode final : SceneNode
{
public:
	LightNode();

	void OnUpdate(float64 dt) override {}
	
	Colour GetColour() const;
	void SetColour(const Colour& colour);
	float32 GetIntensity() const;
	void SetIntensity(const float32 intensity);
	LightType GetLightType() const;
	void SetLightType(const LightType lightType);
private:
	Colour _colour;
	float32 _intensity;
	LightType _lightType;
};
