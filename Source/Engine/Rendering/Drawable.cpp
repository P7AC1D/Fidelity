#include "Drawable.h"

#include <cmath>

#include "../Core/Maths.h"
#include "../UI/ImGui/imgui.h"
#include "../UI/UiManager.hpp"
#include "StaticMesh.h"
#include "Material.h"

Drawable::Drawable() : Component(ComponentType::Drawable),
                       _currentScale(Vector3::Identity),
                       _currentRotationEuler(Vector3::Zero),
                       _drawAabb(false),
                       _modified(true)
{
}

void Drawable::drawInspector()
{
  ImGui::Separator();
  ImGui::Text("Drawable");
	std::shared_ptr<Material> material = getMaterial();
	{
		Colour ambient = material->getAmbientColour();
		float32 rawCol[3] = { ambient[0], ambient[1], ambient[2] };
		ImGui::ColorEdit3("Ambient", rawCol);
		material->setAmbientColour(Colour(rawCol[0] * 255, rawCol[1] * 255, rawCol[2] * 255));
	}
	{
		Colour diffuse = material->getDiffuseColour();
		float32 rawCol[3] = { diffuse[0], diffuse[1], diffuse[2] };
		ImGui::ColorEdit3("Diffuse", rawCol);
		material->setDiffuseColour(Colour(rawCol[0] * 255, rawCol[1] * 255, rawCol[2] * 255));
	}
	{
		Colour specular = material->getSpecularColour();
		float32 rawCol[3] = { specular[0], specular[1], specular[2] };
		ImGui::ColorEdit3("Specular", rawCol);
		material->setSpecularColour(Colour(rawCol[0] * 255, rawCol[1] * 255, rawCol[2] * 255));
	}
	{
		float32 specular = material->getSpecularExponent();
		ImGui::DragFloat("Exponent", &specular, 1.0f, 0.0f, 1000.0f);
		material->setSpecularExponent(specular);
	}
	{
		std::vector<const char*> debugRenderingItems = { "Diffuse", "Normal", "Specular" };
		static int debugRenderingCurrentItem = 0;

		ImGui::Combo("Texture", &debugRenderingCurrentItem, debugRenderingItems.data(), debugRenderingItems.size());
		if (debugRenderingCurrentItem == 0)
		{
			auto diffuseTexture = material->getDiffuseTexture();
			if (diffuseTexture == nullptr)
			{
				return;
			}

			UiManager::AddTexture(reinterpret_cast<uint64>(&diffuseTexture), diffuseTexture);
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

			UiManager::AddTexture(reinterpret_cast<uint64>(&normalTexture), normalTexture);
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
			auto specularTexture = material->getSpecularTexture();
			if (specularTexture == nullptr)
			{
				return;
			}

			UiManager::AddTexture(reinterpret_cast<uint64>(&specularTexture), specularTexture);
			ImGui::Image(
				&specularTexture,
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
    if (_currentRotationEuler != _rotation.ToEuler() || _currentScale != _scale)
    {
      updateAabb(_scale, _rotation);
      _currentScale = _scale;
      _currentRotationEuler = _rotation.ToEuler();
    }

    Matrix4 translation = Matrix4::Translation(_position);
    Matrix4 scale = Matrix4::Scaling(_scale);
    Matrix4 rotation = Matrix4::Rotation(_rotation);
    _transform = translation * scale * rotation;

    _modified = true;
  }
}

void Drawable::onNotify(const GameObject &gameObject)
{
  Transform transform(gameObject.getTransform());
  _scale = transform.getScale();
  _rotation = transform.getRotation();
  _position = transform.getPosition();
  _modified = true;
}

void Drawable::updateAabb(Vector3 scalingDelta, Quaternion rotationDelta)
{
  Vector3 min(_initAabb.GetNegBounds());
  Vector3 max(_initAabb.GetPosBounds());
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