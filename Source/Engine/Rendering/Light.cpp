#include "Light.h"

#include "../UI/ImGui/imgui.h"
#include "../Core/GameObject.h"

Light::Light() : Component(ComponentType::Light),
								 _colour(Colour::White),
								 _radius(10.0f),
								 _lightType(LightType::Point),
								 _modified(true),
								 _direction(Vector3::Identity),
								 _intensity(100.0f),
								 // Initialize shadow properties
								 _castsShadows(false),
								 _shadowResolution(1024),
								 _shadowNearPlane(0.1f),
								 _shadowFarPlane(100.0f)
{
}

void Light::drawInspector()
{
	if (ImGui::CollapsingHeader("Light"))
	{
		float32 rawCol[]{_colour[0], _colour[1], _colour[2]};
		ImGui::ColorEdit3("Colour", rawCol);
		setColour(Colour(rawCol[0] * 255, rawCol[1] * 255, rawCol[2] * 255));

		float32 radius = _radius;
		if (_lightType == LightType::Point)
		{
			if (ImGui::SliderFloat("Radius", &radius, 0.0f, 200.0f))
			{
				setRadius(radius);
			}
		}

		float32 intensity = _intensity;
		if (ImGui::SliderFloat("Intensity", &intensity, 0.0f, 100.0f))
		{
			setIntensity(intensity);
		}

		// Shadow Settings Section
		ImGui::Separator();
		ImGui::Text("Shadow Settings");
		
		bool castsShadows = _castsShadows;
		if (ImGui::Checkbox("Cast Shadows", &castsShadows))
		{
			setCastsShadows(castsShadows);
		}

		// Only show shadow controls when shadows are enabled
		if (_castsShadows)
		{
			// Shadow resolution dropdown
			const char* resolutionOptions[] = { "256", "512", "1024", "2048", "4096" };
			int currentResIndex = 2; // default to 1024
			uint32 resolutions[] = { 256, 512, 1024, 2048, 4096 };
			
			// Find current resolution index
			for (int i = 0; i < 5; i++)
			{
				if (resolutions[i] == _shadowResolution)
				{
					currentResIndex = i;
					break;
				}
			}
			
			if (ImGui::Combo("Shadow Resolution", &currentResIndex, resolutionOptions, 5))
			{
				setShadowResolution(resolutions[currentResIndex]);
			}

			// Near and far plane controls for point lights
			if (_lightType == LightType::Point)
			{
				float32 nearPlane = _shadowNearPlane;
				if (ImGui::SliderFloat("Shadow Near Plane", &nearPlane, 0.01f, 10.0f))
				{
					setShadowNearPlane(nearPlane);
				}

				float32 farPlane = _shadowFarPlane;
				if (ImGui::SliderFloat("Shadow Far Plane", &farPlane, 1.0f, 500.0f))
				{
					setShadowFarPlane(farPlane);
				}
			}
		}
	}
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

Light &Light::setIntensity(float32 intensity)
{
	_intensity = intensity;
	_modified = true;
	return *this;
}

// Shadow property setters
Light &Light::setCastsShadows(bool castsShadows)
{
	_castsShadows = castsShadows;
	_modified = true;
	return *this;
}

Light &Light::setShadowResolution(uint32 resolution)
{
	_shadowResolution = resolution;
	_modified = true;
	return *this;
}

Light &Light::setShadowNearPlane(float32 nearPlane)
{
	_shadowNearPlane = nearPlane;
	_modified = true;
	return *this;
}

Light &Light::setShadowFarPlane(float32 farPlane)
{
	_shadowFarPlane = farPlane;
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
	Transform transform = gameObject.getGlobalTransform();
	_position = transform.getPosition();
	_rotation = transform.getRotation();
	_direction = _rotation.Rotate(Vector3(0, -1, 0));
	_direction.Normalize();

	_modified = true;
}