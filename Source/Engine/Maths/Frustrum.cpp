#include "Frustrum.hpp"
#include "../Core/TransformComponent.h"
#include "../Rendering/CameraComponent.h"
#include "Vector3.hpp"
#include <cmath>

Frustrum::Frustrum()
{
  // Create a default frustum that contains everything
  // This ensures backward compatibility for uninitialized frustums
  // The Plane constructor expects (normal, point_on_plane)
  _left = Plane(Vector3(1.0f, 0.0f, 0.0f), Vector3(-1000.0f, 0.0f, 0.0f));
  _right = Plane(Vector3(-1.0f, 0.0f, 0.0f), Vector3(1000.0f, 0.0f, 0.0f));
  _top = Plane(Vector3(0.0f, -1.0f, 0.0f), Vector3(0.0f, 1000.0f, 0.0f));
  _bottom = Plane(Vector3(0.0f, 1.0f, 0.0f), Vector3(0.0f, -1000.0f, 0.0f));
  _near = Plane(Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 0.0f, -0.01f));
  _far = Plane(Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, 0.0f, 1000.0f));
}

Frustrum::Frustrum(const CameraComponent &camera)
{
  // Extract frustum planes from camera's view-projection matrix
  Matrix4 viewMatrix = camera.getView();
  Matrix4 projMatrix = camera.getProj();

  // CRITICAL FIX: Check if we're in a fallback situation more thoroughly
  // If the view matrix appears to be a default/fallback matrix, use conservative culling
  bool isViewMatrixIdentity = (viewMatrix == Matrix4::Identity);
  bool isViewMatrixDefault = false;

  // Check if this is our new default look-at matrix (0,0,5) -> (0,0,0)
  Vector3 expectedEye(0, 0, 5);
  Vector3 expectedTarget(0, 0, 0);
  Vector3 expectedUp(0, 1, 0);
  Matrix4 expectedDefault = Matrix4::LookAt(expectedEye, expectedTarget, expectedUp);
  isViewMatrixDefault = (viewMatrix == expectedDefault);

  if (isViewMatrixIdentity || isViewMatrixDefault)
  {
    // OLD FALLBACK CODE - DISABLED TO USE CORRECTED VERSION BELOW
    /*
    float32 near = camera.getNear();
    float32 far = camera.getFar();

    // Create proper frustum planes for a camera at origin looking down -Z
    // Use standard perspective frustum math
    float32 fov = camera.getFov().InRadians();
    float32 aspect = camera.getAspectRatio();

    float32 nearHeight = 2.0f * tan(fov * 0.5f) * near;
    float32 nearWidth = nearHeight * aspect;
    float32 farHeight = 2.0f * tan(fov * 0.5f) * far;
    float32 farWidth = farHeight * aspect;

    // Create normalized plane equations for proper culling
    // Left plane: points inside frustum have positive distance
    _left = Plane(Vector3(cos(atan(nearWidth / (2.0f * near))), 0.0f, sin(atan(nearWidth / (2.0f * near)))), Vector3::Zero);
    _right = Plane(Vector3(-cos(atan(nearWidth / (2.0f * near))), 0.0f, sin(atan(nearWidth / (2.0f * near)))), Vector3::Zero);
    _top = Plane(Vector3(0.0f, -cos(atan(nearHeight / (2.0f * near))), sin(atan(nearHeight / (2.0f * near)))), Vector3::Zero);
    _bottom = Plane(Vector3(0.0f, cos(atan(nearHeight / (2.0f * near))), sin(atan(nearHeight / (2.0f * near)))), Vector3::Zero);

    // CRITICAL: Near and far planes must properly exclude objects behind camera
    _near = Plane(Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 0.0f, -near));  // Points toward camera
    _far = Plane(Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, 0.0f, -far));   // Points away from camera

    return;
    */
  }

  // CORRECT FRUSTUM CONSTRUCTION - using proper geometric approach
  // Camera transforms are verified correct, Plane class works correctly

  float32 fov = camera.getFov().InRadians();
  float32 aspect = camera.getAspectRatio();
  float32 near = camera.getNear();
  float32 far = camera.getFar();

  // Get camera world transform (verified correct)
  Vector3 cameraPos = camera.getWorldPosition();
  Vector3 cameraForward = camera.getWorldForward();
  Vector3 cameraUp = camera.getWorldUp();
  Vector3 cameraRight = Vector3::Cross(cameraForward, cameraUp);

  // Calculate frustum geometry
  float32 halfFovY = fov * 0.5f;
  float32 tanHalfFovY = tan(halfFovY);
  float32 tanHalfFovX = tanHalfFovY * aspect;

  // Calculate near and far plane centers
  Vector3 nearCenter = cameraPos + cameraForward * near;
  Vector3 farCenter = cameraPos + cameraForward * far;

  // Calculate corner directions for side planes
  Vector3 nearTopLeft = cameraForward + cameraUp * tanHalfFovY - cameraRight * tanHalfFovX;
  Vector3 nearTopRight = cameraForward + cameraUp * tanHalfFovY + cameraRight * tanHalfFovX;
  Vector3 nearBottomLeft = cameraForward - cameraUp * tanHalfFovY - cameraRight * tanHalfFovX;
  Vector3 nearBottomRight = cameraForward - cameraUp * tanHalfFovY + cameraRight * tanHalfFovX;

  // Create frustum planes with INWARD-pointing normals (toward frustum center)
  // For proper containment testing, all normals must point INWARD

  // Left plane: normal points toward the RIGHT (inward toward frustum center)
  Vector3 leftNormal = Vector3::Normalize(Vector3::Cross(nearBottomLeft, nearTopLeft));
  _left = Plane(leftNormal, cameraPos);

  // Right plane: normal points toward the LEFT (inward toward frustum center)
  Vector3 rightNormal = Vector3::Normalize(Vector3::Cross(nearTopRight, nearBottomRight));
  _right = Plane(rightNormal, cameraPos);

  // Top plane: normal points DOWNWARD (inward toward frustum center)
  Vector3 topNormal = Vector3::Normalize(Vector3::Cross(nearTopLeft, nearTopRight));
  _top = Plane(topNormal, cameraPos);

  // Bottom plane: normal points UPWARD (inward toward frustum center)
  Vector3 bottomNormal = Vector3::Normalize(Vector3::Cross(nearBottomRight, nearBottomLeft));
  _bottom = Plane(bottomNormal, cameraPos);

  // Near plane: normal points away from camera (toward objects in frustum)
  _near = Plane(cameraForward, nearCenter);

  // Far plane: normal points toward camera (toward objects in frustum)
  _far = Plane(-cameraForward, farCenter);

  return;
}

Frustrum::Frustrum(const Plane &left, const Plane &right, const Plane &top,
                   const Plane &bottom, const Plane &nearPlane, const Plane &farPlane)
    : _left(left), _right(right), _top(top), _bottom(bottom), _near(nearPlane), _far(farPlane)
{
}

bool Frustrum::contains(const Aabb &box, const TransformComponent &transform) const
{
  // Get transform matrix from component
  Matrix4 transformMatrix = transform.getWorldMatrix();

  // Check if transform is axis-aligned for optimization
  if (isTransformAxisAligned(transform))
  {
    return containsAxisAligned(box, transform);
  }
  else
  {
    return containsOriented(box, transform);
  }
}

bool Frustrum::contains(const Aabb &box, const Matrix4 &transform) const
{
  // Check if the Matrix4 represents an axis-aligned transform (translation + uniform scale only)
  if (isMatrix4AxisAligned(transform))
  {
    // Use the optimized axis-aligned path
    return containsAxisAlignedMatrix4(box, transform);
  }
  else
  {
    // For arbitrary transforms, use the oriented bounding box approach
    return containsOrientedMatrix4(box, transform);
  }
}

bool Frustrum::isTransformAxisAligned(const TransformComponent &transform) const
{
  // A transform is axis-aligned if it only has translation and uniform scale
  Vector3 scale = transform.getScale();
  Quaternion rotation = transform.getRotation();

  // Check if rotation is identity (or very close to it)
  Quaternion identity = Quaternion::Identity;
  float32 dotProduct = abs(rotation.W * identity.W + rotation.X * identity.X +
                           rotation.Y * identity.Y + rotation.Z * identity.Z);

  return dotProduct > 0.999f; // Very close to identity
}

bool Frustrum::containsAxisAligned(const Aabb &aabb, const TransformComponent &transform) const
{
  // For axis-aligned transforms, we can transform the AABB directly
  Vector3 position = transform.getPosition();
  Vector3 scale = transform.getScale();

  // Transform AABB
  Vector3 min = aabb.getMin() * scale + position;
  Vector3 max = aabb.getMax() * scale + position;

  // Ensure min/max are correct after scaling
  if (scale.X < 0.0f)
  {
    float32 temp = min.X;
    min.X = max.X;
    max.X = temp;
  }
  if (scale.Y < 0.0f)
  {
    float32 temp = min.Y;
    min.Y = max.Y;
    max.Y = temp;
  }
  if (scale.Z < 0.0f)
  {
    float32 temp = min.Z;
    min.Z = max.Z;
    max.Z = temp;
  }

  // Test against each frustum plane
  return testAABBAgainstPlane(min, max, _left) &&
         testAABBAgainstPlane(min, max, _right) &&
         testAABBAgainstPlane(min, max, _top) &&
         testAABBAgainstPlane(min, max, _bottom) &&
         testAABBAgainstPlane(min, max, _near) &&
         testAABBAgainstPlane(min, max, _far);
}

bool Frustrum::containsOriented(const Aabb &aabb, const TransformComponent &transform) const
{
  // For oriented transforms, we need to test all 8 corners of the AABB
  Matrix4 transformMatrix = transform.getWorldMatrix();

  Vector3 min = aabb.getMin();
  Vector3 max = aabb.getMax();

  // Generate all 8 corners of the AABB
  Vector3 corners[8] = {
      {min.X, min.Y, min.Z},
      {max.X, min.Y, min.Z},
      {min.X, max.Y, min.Z},
      {max.X, max.Y, min.Z},
      {min.X, min.Y, max.Z},
      {max.X, min.Y, max.Z},
      {min.X, max.Y, max.Z},
      {max.X, max.Y, max.Z}};

  // Transform all corners
  Vector3 transformedCorners[8];
  for (int i = 0; i < 8; i++)
  {
    transformedCorners[i] = transformMatrix * corners[i];
  }

  // Test against each frustum plane
  Plane planes[6] = {_left, _right, _top, _bottom, _near, _far};

  for (int p = 0; p < 6; p++)
  {
    bool allOutside = true;
    for (int c = 0; c < 8; c++)
    {
      if (planes[p].getSignedDistance(transformedCorners[c]) >= 0.0f)
      {
        allOutside = false;
        break;
      }
    }

    // If all corners are outside this plane, the AABB is outside the frustum
    if (allOutside)
    {
      return false;
    }
  }

  return true;
}

bool Frustrum::testAABBAgainstPlane(const Vector3 &min, const Vector3 &max, const Plane &plane) const
{
  // Find the positive vertex (vertex most in the direction of the plane normal)
  Vector3 normal = plane.getNormal();
  Vector3 positiveVertex = min;

  if (normal.X >= 0.0f)
    positiveVertex.X = max.X;
  if (normal.Y >= 0.0f)
    positiveVertex.Y = max.Y;
  if (normal.Z >= 0.0f)
    positiveVertex.Z = max.Z;

  // If the positive vertex is behind the plane, the AABB is completely outside
  return plane.getSignedDistance(positiveVertex) >= 0.0f;
}

void Frustrum::extractPlanesFromMatrix(const Matrix4 &viewProjMatrix)
{
  // Extract frustum planes from view-projection matrix using Gribb-Hartmann method
  // Matrix4 uses Vector4[4] array accessible via operator[]
  const Vector4 &row0 = viewProjMatrix[0];
  const Vector4 &row1 = viewProjMatrix[1];
  const Vector4 &row2 = viewProjMatrix[2];
  const Vector4 &row3 = viewProjMatrix[3];

  // CRITICAL FIX: With transpose working for depth, try negating side planes for correct orientation

  // Left plane: -(row3 + row0) - flip to get correct inward normal
  Vector4 leftPlane = Vector4(-(row3.X + row0.X), -(row3.Y + row0.Y), -(row3.Z + row0.Z), -(row3.W + row0.W));
  _left = createPlaneFromVector4(leftPlane);

  // Right plane: -(row3 - row0) - flip to get correct inward normal
  Vector4 rightPlane = Vector4(-(row3.X - row0.X), -(row3.Y - row0.Y), -(row3.Z - row0.Z), -(row3.W - row0.W));
  _right = createPlaneFromVector4(rightPlane);

  // Bottom plane: -(row3 + row1) - flip to get correct inward normal
  Vector4 bottomPlane = Vector4(-(row3.X + row1.X), -(row3.Y + row1.Y), -(row3.Z + row1.Z), -(row3.W + row1.W));
  _bottom = createPlaneFromVector4(bottomPlane);

  // Top plane: -(row3 - row1) - flip to get correct inward normal
  Vector4 topPlane = Vector4(-(row3.X - row1.X), -(row3.Y - row1.Y), -(row3.Z - row1.Z), -(row3.W - row1.W));
  _top = createPlaneFromVector4(topPlane);

  // Near/Far planes are working with transpose, keep them as-is
  Vector4 nearPlane = Vector4(row3.X + row2.X, row3.Y + row2.Y, row3.Z + row2.Z, row3.W + row2.W);
  _near = createPlaneFromVector4(nearPlane);

  Vector4 farPlane = Vector4(row3.X - row2.X, row3.Y - row2.Y, row3.Z - row2.Z, row3.W - row2.W);
  _far = createPlaneFromVector4(farPlane);
}

Plane Frustrum::createPlaneFromVector4(const Vector4 &planeVector) const
{
  // The Vector4 represents the plane equation: ax + by + cz + d = 0
  // We need to normalize it first
  Vector3 normal(planeVector.X, planeVector.Y, planeVector.Z);
  float32 distance = planeVector.W;

  // Normalize the plane
  float32 length = Vector3::Length(normal);
  if (length > 0.0001f) // Avoid division by zero
  {
    normal /= length;
    distance /= length;
  }

  // Create a plane directly using the normal and distance
  // The Plane constructor expects normal and a point on the plane
  // For plane equation ax + by + cz + d = 0, a point on the plane is normal * (-d)
  Vector3 pointOnPlane = normal * (-distance);

  return Plane(normal, pointOnPlane);
}

bool Frustrum::isMatrix4AxisAligned(const Matrix4 &transform) const
{
  // A Matrix4 is axis-aligned if:
  // 1. The rotation part (upper-left 3x3) is identity or close to it
  // 2. No skew or shear components exist

  const float32 EPSILON = 0.0001f;

  // Check if the 3x3 rotation matrix is axis-aligned (diagonal with possible scale)
  // For axis-aligned, off-diagonal elements should be near zero
  if (abs(transform[0][1]) > EPSILON || abs(transform[0][2]) > EPSILON ||
      abs(transform[1][0]) > EPSILON || abs(transform[1][2]) > EPSILON ||
      abs(transform[2][0]) > EPSILON || abs(transform[2][1]) > EPSILON)
  {
    return false;
  }

  // Check that w-row is correct for affine transform [0, 0, 0, 1]
  if (abs(transform[3][0]) > EPSILON || abs(transform[3][1]) > EPSILON ||
      abs(transform[3][2]) > EPSILON || abs(transform[3][3] - 1.0f) > EPSILON)
  {
    return false;
  }

  return true;
}

bool Frustrum::containsAxisAlignedMatrix4(const Aabb &aabb, const Matrix4 &transform) const
{
  // Extract translation and scale from the axis-aligned Matrix4
  Vector3 translation(transform[0][3], transform[1][3], transform[2][3]);
  Vector3 scale(transform[0][0], transform[1][1], transform[2][2]);

  // Transform AABB using the extracted translation and scale
  Vector3 min = aabb.getMin() * scale + translation;
  Vector3 max = aabb.getMax() * scale + translation;

  // Ensure min/max are correct after scaling (handle negative scale)
  if (scale.X < 0.0f)
  {
    float32 temp = min.X;
    min.X = max.X;
    max.X = temp;
  }
  if (scale.Y < 0.0f)
  {
    float32 temp = min.Y;
    min.Y = max.Y;
    max.Y = temp;
  }
  if (scale.Z < 0.0f)
  {
    float32 temp = min.Z;
    min.Z = max.Z;
    max.Z = temp;
  }

  // Test against each frustum plane using the optimized AABB-plane test
  return testAABBAgainstPlane(min, max, _left) &&
         testAABBAgainstPlane(min, max, _right) &&
         testAABBAgainstPlane(min, max, _top) &&
         testAABBAgainstPlane(min, max, _bottom) &&
         testAABBAgainstPlane(min, max, _near) &&
         testAABBAgainstPlane(min, max, _far);
}

bool Frustrum::containsOrientedMatrix4(const Aabb &box, const Matrix4 &transform) const
{
  // For arbitrary transforms, use the oriented bounding box approach
  // This is the original implementation moved to a separate function

  // Get AABB min/max and compute the 8 corners
  Vector3 min = box.getMin();
  Vector3 max = box.getMax();

  Vector3 corners[8] = {
      Vector3(min.X, min.Y, min.Z), // 0: min corner
      Vector3(max.X, min.Y, min.Z), // 1: max X
      Vector3(min.X, max.Y, min.Z), // 2: max Y
      Vector3(max.X, max.Y, min.Z), // 3: max X,Y
      Vector3(min.X, min.Y, max.Z), // 4: max Z
      Vector3(max.X, min.Y, max.Z), // 5: max X,Z
      Vector3(min.X, max.Y, max.Z), // 6: max Y,Z
      Vector3(max.X, max.Y, max.Z)  // 7: max corner
  };

  // Transform all corners
  for (int i = 0; i < 8; ++i)
  {
    Vector4 corner4D(corners[i].X, corners[i].Y, corners[i].Z, 1.0f);
    Vector4 transformedCorner = transform * corner4D;
    corners[i] = Vector3(transformedCorner.X, transformedCorner.Y, transformedCorner.Z);
  }

  // Test the transformed AABB against each frustum plane using correct algorithm
  // FIXED: Use separating axis theorem - if ALL corners are outside ANY plane,
  // then the AABB is completely outside the frustum

  Plane planes[6] = {_left, _right, _top, _bottom, _near, _far};

  for (int p = 0; p < 6; p++)
  {
    bool allCornersOutside = true;

    // Check if all corners are outside this plane
    for (int i = 0; i < 8; i++)
    {
      if (planes[p].getSignedDistance(corners[i]) >= 0.0f)
      {
        allCornersOutside = false;
        break; // At least one corner is inside this plane
      }
    }

    // If all corners are outside any plane, the AABB is outside the frustum
    if (allCornersOutside)
    {
      return false;
    }
  }

  // If we reach here, the AABB intersects or is inside the frustum
  return true;
}

const Plane &Frustrum::getPlane(int index) const
{
  switch (index)
  {
  case 0:
    return _left;
  case 1:
    return _right;
  case 2:
    return _top;
  case 3:
    return _bottom;
  case 4:
    return _near;
  case 5:
    return _far;
  default:
    return _left; // Fallback to avoid undefined behavior
  }
}

std::array<Plane, 6> Frustrum::getPlanes() const
{
  return {_left, _right, _top, _bottom, _near, _far};
}

bool Frustrum::isValid() const
{
  // Check if all planes have valid (non-zero) normals
  const float32 MIN_NORMAL_LENGTH = 0.0001f;

  return (Vector3::Length(_left.getNormal()) > MIN_NORMAL_LENGTH &&
          Vector3::Length(_right.getNormal()) > MIN_NORMAL_LENGTH &&
          Vector3::Length(_top.getNormal()) > MIN_NORMAL_LENGTH &&
          Vector3::Length(_bottom.getNormal()) > MIN_NORMAL_LENGTH &&
          Vector3::Length(_near.getNormal()) > MIN_NORMAL_LENGTH &&
          Vector3::Length(_far.getNormal()) > MIN_NORMAL_LENGTH);
}

bool Frustrum::contains(const Vector3 &point) const
{
  // A point is inside the frustum if it's on the positive side of all planes
  return (_left.getSignedDistance(point) >= 0.0f &&
          _right.getSignedDistance(point) >= 0.0f &&
          _top.getSignedDistance(point) >= 0.0f &&
          _bottom.getSignedDistance(point) >= 0.0f &&
          _near.getSignedDistance(point) >= 0.0f &&
          _far.getSignedDistance(point) >= 0.0f);
}

float32 Frustrum::getVolume() const
{
  // This is a simplified volume calculation for the frustum
  // For a perspective frustum, this would need more complex geometry
  // For now, we'll estimate using the distance between near and far planes

  Vector3 nearPoint = _near.getNormal() * (-_near.getDistance(_near.getNormal() * 0.0f));
  Vector3 farPoint = _far.getNormal() * (-_far.getDistance(_far.getNormal() * 0.0f));

  float32 depth = Vector3::Length(farPoint - nearPoint);

  // Rough approximation - for a proper calculation, we'd need more geometric analysis
  return depth * 1000.0f; // Placeholder calculation
}
