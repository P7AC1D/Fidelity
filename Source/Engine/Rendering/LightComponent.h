#pragma once
#include "../Core/ComponentBase.h"
#include "../Core/ComponentTypeId.h"
#include "../Core/Maths.h"
#include <memory>

// Forward declarations
class TransformComponent;
class PointLightCuller;

enum class LightComponentType
{
  Point,
  Directional,
  Spot,
  Area
};

/// Per-light culling statistics for point lights
struct PointLightCullingStats
{
  uint32 totalObjects = 0;
  uint32 objectsFailedSphere = 0;
  uint32 objectsFailedFace = 0;
  uint32 finalRenderedObjects = 0;
  float sphereCullingRatio = 0.0f;
  float faceCullingRatio = 0.0f;
  bool hasValidStats = false;

  void reset()
  {
    totalObjects = objectsFailedSphere = objectsFailedFace = finalRenderedObjects = 0;
    sphereCullingRatio = faceCullingRatio = 0.0f;
    hasValidStats = false;
  }
};

/// Modern Light component that uses ComponentBase.
/// Handles all light types with shadow support and proper transform integration.
/// Queries TransformComponent directly from parent GameObject.
class LightComponent : public ComponentBase
{
public:
  LightComponent();
  LightComponent(LightComponentType lightType, const Colour &colour = Colour::White, float32 intensity = 1000.0f);
  ~LightComponent();

  // ComponentBase interface
  ComponentTypeId getTypeId() const override { return getComponentTypeId<LightComponent>(); }
  void drawInspector() override;

  // Static type information
  static ComponentTypeId GetTypeId() { return getComponentTypeId<LightComponent>(); }

  // Light configuration methods (fluent interface)
  LightComponent &setColour(const Colour &colour);
  LightComponent &setRadius(float32 radius);
  LightComponent &setLightType(LightComponentType lightType);
  LightComponent &setIntensity(float32 intensity);

  // Shadow system support
  LightComponent &setCastsShadows(bool castsShadows);
  LightComponent &setShadowResolution(uint32 resolution);
  LightComponent &setShadowNearPlane(float32 nearPlane);
  LightComponent &setShadowFarPlane(float32 farPlane);

  // Getters
  Matrix4 getMatrix() const { return _matrix; }
  Vector3 getPosition() const;
  Colour getColour() const { return _colour; }
  float32 getRadius() const { return _radius; }
  LightComponentType getLightType() const { return _lightType; }
  Vector3 getDirection() const { return _direction; }
  float32 getIntensity() const { return _intensity; }

  // Shadow getters
  bool getCastsShadows() const { return _castsShadows; }
  uint32 getShadowResolution() const { return _shadowResolution; }
  float32 getShadowNearPlane() const { return _shadowNearPlane; }
  float32 getShadowFarPlane() const { return _shadowFarPlane; }

  // Culling statistics (for point lights)
  void setCullingStats(const void *cullingResult);
  const PointLightCullingStats &getCullingStats() const { return _cullingStats; }

protected:
  // ComponentBase hooks
  void onInitialize() override;
  void onActivate() override;
  void onUpdate(float32 dt) override;

private:
  void updateFromTransform();
  void recalculateMatrix();

  // Light properties
  Colour _colour;
  float32 _radius;
  LightComponentType _lightType;
  Matrix4 _matrix;
  Vector3 _direction;
  float32 _intensity;

  // Shadow properties
  bool _castsShadows;
  uint32 _shadowResolution;
  float32 _shadowNearPlane;
  float32 _shadowFarPlane;

  // Culling statistics (for point lights)
  PointLightCullingStats _cullingStats;

  // State tracking
  bool _modified;
};
