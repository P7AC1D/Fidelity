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
    float32 pos[3] = { _position.X, _position.Y, _position.Z };
    if (ImGui::DragFloat3("Position", pos, 0.1f))
    {
      setPosition(Vector3(pos[0], pos[1], pos[2]));
    }

    // Scale controls
    float32 scl[3] = { _scale.X, _scale.Y, _scale.Z };
    if (ImGui::DragFloat3("Scale", scl, 0.001f))
    {
      setScale(Vector3(scl[0], scl[1], scl[2]));
    }

    // Rotation controls (using Euler angles for easier editing)
    auto euler = _rotation.ToEuler();
    float32 angles[3] = { euler[0].InDegrees(), euler[1].InDegrees(), euler[2].InDegrees() };
    if (ImGui::DragFloat3("Orientation", angles, 1.0f, -180.0f, 180.0f))
    {
      // Convert back to quaternion
      // Note: Using the same order as in your original code (YXZ)
      Quaternion xRot(Vector3(1.0f, 0.0f, 0.0f), Degree(angles[0]).InRadians());
      Quaternion yRot(Vector3(0.0f, 1.0f, 0.0f), Degree(angles[1]).InRadians());
      Quaternion zRot(Vector3(0.0f, 0.0f, 1.0f), Degree(angles[2]).InRadians());
      setRotation(yRot * xRot * zRot);
    }
  }
}

void TransformComponent::setPosition(const Vector3 &position)
{
  if (_position != position)
  {
    _position = position;
    markDirty();
    notifyChanged(); // Notify observers of change
  }
}

void TransformComponent::setRotation(const Quaternion &rotation)
{
  if (_rotation != rotation)
  {
    _rotation = rotation;
    markDirty();
    notifyChanged(); // Notify observers of change
  }
}

void TransformComponent::setScale(const Vector3 &scale)
{
  if (_scale != scale)
  {
    _scale = scale;
    markDirty();
    notifyChanged(); // Notify observers of change
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
  if (_worldMatrixDirty)
  {
    updateWorldMatrix();
  }
  return _worldMatrix;
}

void TransformComponent::setWorldMatrix(const Matrix4 &matrix)
{
  _worldMatrix = matrix;
  _worldMatrixDirty = false;

  // TODO: Extract position, rotation, scale from matrix
  // This would require matrix decomposition
  markDirty();
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

  _worldMatrixDirty = false;
}

TransformComponent::CallbackId TransformComponent::addChangeObserver(ChangeCallback callback)
{
  CallbackId id = _nextCallbackId++;
  _changeObservers.emplace_back(id, std::move(callback));
  return id;
}

void TransformComponent::removeChangeObserver(CallbackId callbackId)
{
  _changeObservers.erase(
    std::remove_if(_changeObservers.begin(), _changeObservers.end(),
      [callbackId](const auto& pair) { return pair.first == callbackId; }),
    _changeObservers.end());
}

void TransformComponent::notifyChanged()
{
  for (const auto &[id, callback] : _changeObservers)
  {
    callback();
  }
}
