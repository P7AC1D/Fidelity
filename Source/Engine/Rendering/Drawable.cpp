#include "Drawable.h"

#include <cmath>

#include "../Core/Maths.h"
#include "../UI/ImGui/imgui.h"
#include "../UI/UiManager.hpp"
#include "StaticMesh.h"
#include "Material.h"

Drawable::Drawable() : Component(ComponentType::Drawable),
											 _currentScale(Vector3::Identity),
											 _drawAabb(false),
											 _modified(true),
											 _scale(Vector3::Identity),
											 _position(Vector3::Zero),
											 _rotation(Quaternion::Zero)
{
	_currentRotationEuler[0] = Radian(0.0f);
	_currentRotationEuler[1] = Radian(0.0f);
	_currentRotationEuler[2] = Radian(0.0f);
}

void Drawable::drawInspector()
{
	if (ImGui::CollapsingHeader("Drawable"))
	{
		std::shared_ptr<Material> material = getMaterial();
		Colour diffuse = material->getDiffuseColour();
		float32 rawCol[3] = {diffuse[0], diffuse[1], diffuse[2]};
		ImGui::ColorEdit3("Diffuse", rawCol);
		material->setDiffuseColour(Colour(rawCol[0] * 255, rawCol[1] * 255, rawCol[2] * 255));

		ImGui::Separator();
		ImGui::Text("PBR Material");
		if (_material->hasDiffuseTexture())
		{
			bool diffuseEnabled = _material->diffuseTextureEnabled();
			if (ImGui::Checkbox("Diffuse Enabled", &diffuseEnabled))
			{
				_material->enableDiffuseTexture(diffuseEnabled);
			}
		}

		if (_material->hasNormalTexture())
		{
			bool normalEnabled = _material->normalTextureEnabled();
			if (ImGui::Checkbox("Normal Enabled", &normalEnabled))
			{
				_material->enableNormalTexture(normalEnabled);
			}
		}

		if (_material->hasMetallicTexture())
		{
			bool metallicEnabled = _material->metallicTextureEnabled();
			if (ImGui::Checkbox("Metallic Enabled", &metallicEnabled))
			{
				_material->enableMetallicTexture(metallicEnabled);
			}
		}

		if (_material->hasRoughnessTexture())
		{
			bool roughnessEnabled = _material->roughnessTextureEnabled();
			if (ImGui::Checkbox("Roughness Enabled", &roughnessEnabled))
			{
				_material->enableRoughnessTexture(roughnessEnabled);
			}
		}

		if (_material->hasOcclusionTexture())
		{
			bool occlusionEnabled = _material->occlusionTextureEnabled();
			if (ImGui::Checkbox("Occlusion Enabled", &occlusionEnabled))
			{
				_material->enableOcclusionTexture(occlusionEnabled);
			}
		}

		if (_material->hasOpacityTexture())
		{
			bool oppacityEnabled = _material->opacityTextureEnabled();
			if (ImGui::Checkbox("Oppacity Enabled", &oppacityEnabled))
			{
				_material->enableOppacityTexture(oppacityEnabled);
			}
		}

		if (!_material->hasMetallicTexture() || !_material->metallicTextureEnabled())
		{
			float32 metalness = material->getMetalness();
			if (ImGui::SliderFloat("Metallic", &metalness, 0.0f, 1.f))
			{
				material->setMetalness(metalness);
			}
		}

		if (!_material->hasRoughnessTexture() || !_material->roughnessTextureEnabled())
		{
			float32 roughness = material->getRoughness();
			if (ImGui::SliderFloat("Roughness", &roughness, 0.0f, 1.f))
			{
				material->setRoughness(roughness);
			}
		}
		std::vector<const char *> debugRenderingItems = {"Albedo", "Normal", "Metallic", "Roughness", "Opacity"};
		static int debugRenderingCurrentItem = 0;

		ImGui::Combo("Texture", &debugRenderingCurrentItem, debugRenderingItems.data(), debugRenderingItems.size());
		if (debugRenderingCurrentItem == 0)
		{
			auto diffuseTexture = material->getDiffuseTexture();
			if (diffuseTexture == nullptr)
			{
				return;
			}

			UiManager::addTexture(reinterpret_cast<uint64>(&diffuseTexture), diffuseTexture);
			ImGui::Image(
					&diffuseTexture,
					ImVec2(200, 200),
					ImVec2(0.0f, 0.0f),
					ImVec2(1.0f, 1.0f),
					ImVec4(1.0f, 1.0f, 1.0f, 1.0f),
					ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
		}
		else if (debugRenderingCurrentItem == 1)
		{
			auto normalTexture = material->getNormalTexture();
			if (normalTexture == nullptr)
			{
				return;
			}

			UiManager::addTexture(reinterpret_cast<uint64>(&normalTexture), normalTexture);
			ImGui::Image(
					&normalTexture,
					ImVec2(200, 200),
					ImVec2(0.0f, 0.0f),
					ImVec2(1.0f, 1.0f),
					ImVec4(1.0f, 1.0f, 1.0f, 1.0f),
					ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
		}
		else if (debugRenderingCurrentItem == 2)
		{
			auto texture = material->getMetallicTexture();
			if (texture == nullptr)
			{
				return;
			}

			UiManager::addTexture(reinterpret_cast<uint64>(&texture), texture);
			ImGui::Image(
					&texture,
					ImVec2(200, 200),
					ImVec2(0.0f, 0.0f),
					ImVec2(1.0f, 1.0f),
					ImVec4(1.0f, 1.0f, 1.0f, 1.0f),
					ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
		}
		else if (debugRenderingCurrentItem == 3)
		{
			auto texture = material->getRoughnessTexture();
			if (texture == nullptr)
			{
				return;
			}

			UiManager::addTexture(reinterpret_cast<uint64>(&texture), texture);
			ImGui::Image(
					&texture,
					ImVec2(200, 200),
					ImVec2(0.0f, 0.0f),
					ImVec2(1.0f, 1.0f),
					ImVec4(1.0f, 1.0f, 1.0f, 1.0f),
					ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
		}
		else if (debugRenderingCurrentItem == 4)
		{
			auto opacityTexture = material->getOpacityTexture();
			if (opacityTexture == nullptr)
			{
				return;
			}

			UiManager::addTexture(reinterpret_cast<uint64>(&opacityTexture), opacityTexture);
			ImGui::Image(
					&opacityTexture,
					ImVec2(200, 200),
					ImVec2(0.0f, 0.0f),
					ImVec2(1.0f, 1.0f),
					ImVec4(1.0f, 1.0f, 1.0f, 1.0f),
					ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
		}
	}
}

Drawable &Drawable::setMesh(std::shared_ptr<StaticMesh> mesh)
{
	_mesh = mesh;
	_initAabb = mesh->getAabb();
	_currAabb = _initAabb;
	_modified = true;
	return *this;
}

Drawable &Drawable::setMaterial(std::shared_ptr<Material> material)
{
	_material = material;
	return *this;
}

void Drawable::onUpdate(float32 dt)
{
	if (_modified)
	{
		updateAabb(_scale, _rotation);

		Matrix4 translation = Matrix4::Translation(_position);
		Matrix4 scale = Matrix4::Scaling(_scale);
		Matrix4 rotation = Matrix4::Rotation(_rotation);
		_transform = translation * scale * rotation;

		_modified = false;
	}
}

void Drawable::onNotify(const GameObject &gameObject)
{
	Transform transform(gameObject.getGlobalTransform());
	_scale = transform.getScale();
	_rotation = transform.getRotation();
	_position = transform.getPosition();
	_modified = true;
}

void Drawable::updateAabb(Vector3 scalingDelta, Quaternion rotationDelta)
{
	Vector3 min(_initAabb.getMin());
	Vector3 max(_initAabb.getMax());
	Vector3 newMin(0);
	Vector3 newMax(0);

	float32 a, b;
	Matrix4 rotation = Matrix4::Rotation(rotationDelta);
	Matrix4 scaling = Matrix4::Scaling(scalingDelta);
	Matrix3 transform(Matrix4::ToMatrix3(scaling * rotation));

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			a = transform[j][i] * min[j];
			b = transform[j][i] * max[j];

			newMin[i] += std::fminf(a, b);
			newMax[i] += std::fmaxf(a, b);
		}
	}

	_currAabb = Aabb(newMax, newMin);
}