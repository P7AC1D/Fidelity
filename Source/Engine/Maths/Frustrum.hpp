#pragma once
#include <array>
#include <string>
#include "AABB.hpp"
#include "Matrix4.hpp"
#include "Plane.hpp"
#include "Vector4.hpp"

class CameraComponent;
class TransformComponent;

class Frustrum
{
public:
  Frustrum();
  Frustrum(const CameraComponent &camera);
  Frustrum(const Plane &left, const Plane &right, const Plane &top,
           const Plane &bottom, const Plane &nearPlane, const Plane &farPlane);

  bool contains(const Aabb &box, const TransformComponent &transform) const;
  bool contains(const Aabb &box, const Matrix4 &transform) const;

  /// @brief Get a specific frustum plane by index (0=left, 1=right, 2=top, 3=bottom, 4=near, 5=far)
  /// @param index The plane index (0-5)
  /// @return Reference to the specified plane
  const Plane &getPlane(int index) const;

  /// @brief Get all six frustum planes as an array
  /// @return Array of all six frustum planes
  std::array<Plane, 6> getPlanes() const;

  /// @brief Check if the frustum is valid (all planes have valid normals)
  /// @return True if the frustum has valid plane definitions
  bool isValid() const;

  /// @brief Test if a point is inside the frustum
  /// @param point The point to test
  /// @return True if the point is inside the frustum
  bool contains(const Vector3 &point) const;

  /// @brief Calculate the approximate volume of the frustum
  /// @return The volume of the frustum (approximated)
  float32 getVolume() const;

  /// @brief Validate frustum plane orientations for debugging
  /// @return True if all planes have correct inward-pointing normals
  bool validatePlaneOrientations() const;

  /// @brief Get debug information about frustum plane distances from a point
  /// @param point The point to test
  /// @return String containing distance information for debugging
  std::string getDebugDistances(const Vector3 &point) const;

private:
  // Helper methods for optimized culling
  bool containsAxisAligned(const Aabb &aabb, const TransformComponent &transform) const;
  bool containsOriented(const Aabb &aabb, const TransformComponent &transform) const;
  bool isTransformAxisAligned(const TransformComponent &transform) const;

  // Matrix4 optimization helpers
  bool isMatrix4AxisAligned(const Matrix4 &transform) const;
  bool containsAxisAlignedMatrix4(const Aabb &aabb, const Matrix4 &transform) const;
  bool containsOrientedMatrix4(const Aabb &box, const Matrix4 &transform) const;

  // Frustum plane extraction and testing
  void extractPlanesFromMatrix(const Matrix4 &viewProjMatrix);
  bool testAABBAgainstPlane(const Vector3 &min, const Vector3 &max, const Plane &plane) const;
  Plane createPlaneFromVector4(const Vector4 &planeVector) const;

  Plane _left;
  Plane _right;
  Plane _top;
  Plane _bottom;
  Plane _far;
  Plane _near;
};
