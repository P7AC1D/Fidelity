#include "TransformComponent.h"
#include "../Core/Maths.h"
#include "../UI/ImGui/imgui.h"
#include "../Maths/Degree.hpp"
#include "../Maths/Radian.hpp"
#include <algorithm>

TransformComponent::TransformComponent()
    : _position(Vector3::Zero), _rotation(Quaternion::Identity), _scale(Vector3::Identity)
{
}

TransformComponent::TransformComponent(const Vector3 &position, const Quaternion &rotation, const Vector3 &scale)
    : _position(position), _rotation(rotation), _scale(scale)
{
}

void TransformComponent::onInitialize()
{
  // Mark as dirty when component is initialized
  markDirty();
}

ComponentTypeId TransformComponent::getTypeId() const
{
  return GetTypeId();
}

void TransformComponent::drawInspector()
{
  if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
  {
    // Position controls
    float32 pos[3] = {_position.X, _position.Y, _position.Z};
    if (ImGui::DragFloat3("Position", pos, 0.1f))
    {
      setPosition(Vector3(pos[0], pos[1], pos[2]));
    }

    // Scale controls
    float32 scl[3] = {_scale.X, _scale.Y, _scale.Z};
    if (ImGui::DragFloat3("Scale", scl, 0.001f))
    {
      setScale(Vector3(scl[0], scl[1], scl[2]));
    }

    // Rotation controls (using Euler angles for easier editing)
    // Keep UI Euler angles separate to prevent unwanted coupling
    if (!_uiEulerValid)
    {
      auto euler = _rotation.ToEuler();
      _uiEulerAngles = Vector3(euler[0].InDegrees(), euler[1].InDegrees(), euler[2].InDegrees());
      _uiEulerValid = true;
    }

    float32 angles[3] = {_uiEulerAngles.X, _uiEulerAngles.Y, _uiEulerAngles.Z};

    if (ImGui::DragFloat3("Rotation", angles, 1.0f))
    {
      // Clamp pitch to prevent gimbal lock
      angles[0] = std::max(-89.5f, std::min(89.5f, angles[0]));

      // Update UI angles FIRST
      _uiEulerAngles = Vector3(angles[0], angles[1], angles[2]);

      // Convert to quaternion - but don't call setRotation to avoid invalidating UI angles
      Quaternion xRot(Vector3(1.0f, 0.0f, 0.0f), Degree(angles[0]).InRadians());
      Quaternion yRot(Vector3(0.0f, 1.0f, 0.0f), Degree(angles[1]).InRadians());
      Quaternion zRot(Vector3(0.0f, 0.0f, 1.0f), Degree(angles[2]).InRadians());

      // Direct assignment to avoid the setRotation invalidation
      _rotation = yRot * xRot * zRot;
      markDirty();
    }
  }
}

void TransformComponent::setPosition(const Vector3 &position)
{
  if (_position != position)
  {
    _position = position;
    markDirty();
  }
}

void TransformComponent::setRotation(const Quaternion &rotation)
{
  if (_rotation != rotation)
  {
    _rotation = rotation;
    // Only invalidate UI Euler if this change didn't come from the UI
    // (We can detect this by checking if we're currently in drawInspector)
    _uiEulerValid = false;
    markDirty();
  }
}

void TransformComponent::setScale(const Vector3 &scale)
{
  if (_scale != scale)
  {
    _scale = scale;
    markDirty();
  }
}

void TransformComponent::translate(const Vector3 &delta)
{
  setPosition(_position + delta);
}

void TransformComponent::rotate(const Quaternion &rotation)
{
  setRotation(_rotation * rotation);
}

void TransformComponent::scale(const Vector3 &scale)
{
  setScale(Vector3(_scale.X * scale.X, _scale.Y * scale.Y, _scale.Z * scale.Z));
}

const Matrix4 &TransformComponent::getWorldMatrix() const
{
  if (_changeId != _lastMatrixChangeId)
  {
    updateWorldMatrix();
    _lastMatrixChangeId = _changeId;
  }
  return _worldMatrix;
}

void TransformComponent::setWorldMatrix(const Matrix4 &matrix)
{
  _worldMatrix = matrix;

  // TODO: Extract position, rotation, scale from matrix
  // This would require matrix decomposition
  markDirty();                     // We changed the matrix directly, so increment change ID
  _lastMatrixChangeId = _changeId; // Matrix is now up-to-date with current change
}

void TransformComponent::setParent(TransformComponent *parent)
{
  _parent = parent;
  markDirty();
}

void TransformComponent::updateWorldMatrix() const
{
  // Create transform matrix from position, rotation, scale
  Matrix4 scaleMatrix = Matrix4::Scaling(_scale);
  Matrix4 rotationMatrix = Matrix4::Rotation(_rotation);
  Matrix4 translationMatrix = Matrix4::Translation(_position);

  Matrix4 localMatrix = translationMatrix * rotationMatrix * scaleMatrix;

  if (_parent)
  {
    _worldMatrix = _parent->getWorldMatrix() * localMatrix;
  }
  else
  {
    _worldMatrix = localMatrix;
  }

  // Note: _lastMatrixChangeId is updated in getWorldMatrix() after calling this
}
