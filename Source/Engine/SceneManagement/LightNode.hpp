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

class LightNode final : public SceneNode
{
	friend class SceneNode;

public:
	void OnDraw(std::shared_ptr<Renderer> renderer) override;
	void OnUpdate(float64 dt) override {}
	void OnDrawInspector() override;

	LightNode &SetColour(const Colour &colour);
	LightNode &SetIntensity(float32 intensity);
	LightNode &SetLightType(LightType lightType);

	Colour GetColour() const { return _colour; }
	float32 GetIntensity() const { return _intensity; }
	LightType GetLightType() const { return _lightType; }

	SceneNodeType GetNodeType() const override;

protected:
	LightNode();

private:
	Colour _colour;
	float32 _intensity;
	LightType _lightType;
};
