#include "Drawable.h"

#include <cmath>

#include "../Maths/Math.hpp"
#include "StaticMesh.h"
#include "Material.h"

Drawable::Drawable() : _currentScale(Vector3::Identity),
                       _currentRotationEuler(Vector3::Zero),
                       _drawAabb(false)
{
}

void Drawable::update(float32 dt)
{
  auto newRotation = getTransform().getRotation();
  auto newScale = getTransform().getScale();
  if (_currentRotationEuler != newRotation.ToEuler() || _currentScale != newScale)
  {
    updateAabb(newScale, newRotation);
    _currentScale = newScale;
    _currentRotationEuler = newRotation.ToEuler();
  }
}

Drawable &Drawable::setMesh(std::shared_ptr<StaticMesh> mesh)
{
  _mesh = mesh;
  _initAabb = mesh->getAabb();
  _currAabb = _initAabb;
}

Drawable &setMaterial(std::shared_ptr<Material> material)
{
  _material = material;
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