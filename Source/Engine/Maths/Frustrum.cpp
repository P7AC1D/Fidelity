#include "Frustrum.hpp"
#include "../Core/TransformComponent.h"
#include "../Rendering/CameraComponent.h"
#include "Vector3.hpp"
#include <cmath>
#include <cstdio>
#include <string>

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

  // Calculate corner directions for side planes (normalized directions from camera)
  Vector3 nearTopLeft = Vector3::Normalize(cameraForward + cameraUp * tanHalfFovY - cameraRight * tanHalfFovX);
  Vector3 nearTopRight = Vector3::Normalize(cameraForward + cameraUp * tanHalfFovY + cameraRight * tanHalfFovX);
  Vector3 nearBottomLeft = Vector3::Normalize(cameraForward - cameraUp * tanHalfFovY - cameraRight * tanHalfFovX);
  Vector3 nearBottomRight = Vector3::Normalize(cameraForward - cameraUp * tanHalfFovY + cameraRight * tanHalfFovX);

  // Create frustum planes using the STANDARD geometric approach
  // All normals must point toward the interior of the frustum for proper containment testing
  
  // For a perspective frustum, we create planes using two points on each edge
  // and the camera position to define the plane triangles

  // Calculate actual corner points on the near plane
  Vector3 nearTopLeftPoint = nearCenter + cameraUp * (near * tanHalfFovY) - cameraRight * (near * tanHalfFovX);
  Vector3 nearTopRightPoint = nearCenter + cameraUp * (near * tanHalfFovY) + cameraRight * (near * tanHalfFovX);
  Vector3 nearBottomLeftPoint = nearCenter - cameraUp * (near * tanHalfFovY) - cameraRight * (near * tanHalfFovX);
  Vector3 nearBottomRightPoint = nearCenter - cameraUp * (near * tanHalfFovY) + cameraRight * (near * tanHalfFovX);

  // Left plane: formed by camera position, near top-left, and near bottom-left
  // Normal points inward (to the right)
  Vector3 leftEdge1 = nearTopLeftPoint - cameraPos;
  Vector3 leftEdge2 = nearBottomLeftPoint - cameraPos;
  Vector3 leftNormal = Vector3::Normalize(Vector3::Cross(leftEdge2, leftEdge1));
  _left = Plane(leftNormal, cameraPos);

  // Right plane: formed by camera position, near bottom-right, and near top-right  
  // Normal points inward (to the left)
  Vector3 rightEdge1 = nearBottomRightPoint - cameraPos;
  Vector3 rightEdge2 = nearTopRightPoint - cameraPos;
  Vector3 rightNormal = Vector3::Normalize(Vector3::Cross(rightEdge2, rightEdge1));
  _right = Plane(rightNormal, cameraPos);

  // Top plane: formed by camera position, near top-right, and near top-left
  // Normal points inward (downward)
  Vector3 topEdge1 = nearTopRightPoint - cameraPos;
  Vector3 topEdge2 = nearTopLeftPoint - cameraPos;
  Vector3 topNormal = Vector3::Normalize(Vector3::Cross(topEdge2, topEdge1));
  _top = Plane(topNormal, cameraPos);

  // Bottom plane: formed by camera position, near bottom-left, and near bottom-right
  // Normal points inward (upward)
  Vector3 bottomEdge1 = nearBottomLeftPoint - cameraPos;
  Vector3 bottomEdge2 = nearBottomRightPoint - cameraPos;
  Vector3 bottomNormal = Vector3::Normalize(Vector3::Cross(bottomEdge2, bottomEdge1));
  _bottom = Plane(bottomNormal, cameraPos);

  // Near plane: normal points AWAY from camera (toward objects in frustum)
  // Objects closer than near plane should be culled
  _near = Plane(cameraForward, nearCenter);

  // Far plane: normal points TOWARD camera (toward objects in frustum)
  // Objects farther than far plane should be culled
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

  // Check if this is a very small object that might suffer from precision issues
  Vector3 size = max - min;
  const float32 MIN_OBJECT_SIZE = 0.01f; // 1cm minimum size
  bool isVerySmall = (size.X < MIN_OBJECT_SIZE || size.Y < MIN_OBJECT_SIZE || size.Z < MIN_OBJECT_SIZE);
  
  if (isVerySmall)
  {
    // For very small objects, also test the center point to avoid precision culling
    Vector3 center = (min + max) * 0.5f;
    if (contains(center))
    {
      return true; // If center is visible, consider the small object visible
    }
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
    
    // Add epsilon tolerance for oriented AABB testing
    const float32 ORIENTED_EPSILON = 0.0005f;
    
    for (int c = 0; c < 8; c++)
    {
      if (planes[p].getSignedDistance(transformedCorners[c]) >= -ORIENTED_EPSILON)
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

  // Add epsilon tolerance for near-plane precision issues
  // This prevents objects very close to planes from being incorrectly culled
  const float32 PLANE_EPSILON = 0.001f;
  
  // If the positive vertex is behind the plane (with tolerance), the AABB is completely outside
  return plane.getSignedDistance(positiveVertex) >= -PLANE_EPSILON;
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
    
    // Add epsilon tolerance for Matrix4 oriented AABB testing
    const float32 MATRIX_EPSILON = 0.0005f;

    // Check if all corners are outside this plane
    for (int i = 0; i < 8; i++)
    {
      if (planes[p].getSignedDistance(corners[i]) >= -MATRIX_EPSILON)
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
  // Add epsilon tolerance for point containment to handle precision issues
  const float32 POINT_EPSILON = 0.0001f;
  
  // A point is inside the frustum if it's on the positive side of all planes (with tolerance)
  return (_left.getSignedDistance(point) >= -POINT_EPSILON &&
          _right.getSignedDistance(point) >= -POINT_EPSILON &&
          _top.getSignedDistance(point) >= -POINT_EPSILON &&
          _bottom.getSignedDistance(point) >= -POINT_EPSILON &&
          _near.getSignedDistance(point) >= -POINT_EPSILON &&
          _far.getSignedDistance(point) >= -POINT_EPSILON);
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

bool Frustrum::validatePlaneOrientations() const
{
  // Test a point that should be at the center of the frustum
  Vector3 testPoint(0.0f, 0.0f, -10.0f); // 10 units in front of camera
  
  // All planes should have positive distance to a central point
  bool leftValid = _left.getSignedDistance(testPoint) > 0.0f;
  bool rightValid = _right.getSignedDistance(testPoint) > 0.0f;
  bool topValid = _top.getSignedDistance(testPoint) > 0.0f;
  bool bottomValid = _bottom.getSignedDistance(testPoint) > 0.0f;
  bool nearValid = _near.getSignedDistance(testPoint) > 0.0f;
  bool farValid = _far.getSignedDistance(testPoint) > 0.0f;
  
  return leftValid && rightValid && topValid && bottomValid && nearValid && farValid;
}

std::string Frustrum::getDebugDistances(const Vector3 &point) const
{
  char buffer[512];
  snprintf(buffer, sizeof(buffer),
    "Point (%.2f, %.2f, %.2f) distances:\n"
    "Left: %.3f, Right: %.3f, Top: %.3f, Bottom: %.3f, Near: %.3f, Far: %.3f",
    point.X, point.Y, point.Z,
    _left.getSignedDistance(point),
    _right.getSignedDistance(point),
    _top.getSignedDistance(point),
    _bottom.getSignedDistance(point),
    _near.getSignedDistance(point),
    _far.getSignedDistance(point));
  
  return std::string(buffer);
}
