#include "Drawable.h"

#include <cmath>

#include "../Core/Maths.h"
#include "StaticMesh.h"
#include "Material.h"

static const std::string COMPONENT_NAME = "Drawable";

Drawable::Drawable() : Component(COMPONENT_NAME),
                       _currentScale(Vector3::Identity),
                       _currentRotationEuler(Vector3::Zero),
                       _drawAabb(false),
                       _modified(true)
{
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