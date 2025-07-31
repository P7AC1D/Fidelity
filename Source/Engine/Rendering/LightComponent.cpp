#include "LightComponent.h"
#include "../Core/ComponentBase.inl" // For template implementations
#include "../Core/TransformComponent.h"
#include "../Core/GameObject.h"
#include "../UI/ImGui/imgui.h"
#include "PointLightCuller.h"
#include <algorithm> // For std::sort and std::unique

LightComponent::LightComponent()
    : _colour(Colour::White), _radius(10.0f), _lightType(LightComponentType::Point), _matrix(Matrix4::Identity), _direction(Vector3::Identity), _intensity(1000.0f), _castsShadows(false), _shadowResolution(1024), _shadowNearPlane(0.1f), _shadowFarPlane(100.0f), _modified(true)
{
}

LightComponent::LightComponent(LightComponentType lightType, const Colour &colour, float32 intensity)
    : LightComponent() // Delegate to default constructor
{
  _lightType = lightType;
  _colour = colour;
  _intensity = intensity;
}

LightComponent::~LightComponent()
{
  // No cleanup needed for direct query approach
}

void LightComponent::onInitialize()
{
  // Component is ready for use
  _modified = true;
}

void LightComponent::onActivate()
{
  // Mark as modified to ensure initial calculation
  _modified = true;
}

Vector3 LightComponent::getPosition() const
{
  if (auto transform = getComponentShared<TransformComponent>())
  {
    return transform->getPosition();
  }
  return Vector3::Zero;
}

void LightComponent::drawInspector()
{
  if (ImGui::CollapsingHeader("Light Component"))
  {
    float32 rawCol[]{_colour[0], _colour[1], _colour[2]};
    ImGui::ColorEdit3("Colour", rawCol);
    setColour(Colour(rawCol[0] * 255, rawCol[1] * 255, rawCol[2] * 255));

    float32 radius = _radius;
    if (_lightType == LightComponentType::Point)
    {
      if (ImGui::SliderFloat("Radius", &radius, 0.0f, 200.0f))
      {
        setRadius(radius);
      }
    }

    float32 intensity = _intensity;
    if (ImGui::SliderFloat("Intensity", &intensity, 0.0f, 2500.0f))
    {
      setIntensity(intensity);
    }

    // Light type selection
    const char *lightTypeNames[] = {"Point", "Directional", "Spot", "Area"};
    int currentType = static_cast<int>(_lightType);
    if (ImGui::Combo("Light Type", &currentType, lightTypeNames, 4))
    {
      setLightType(static_cast<LightComponentType>(currentType));
    }

    // Shadow Settings Section
    ImGui::Separator();
    ImGui::Text("Shadow Settings");

    bool castsShadows = _castsShadows;
    if (ImGui::Checkbox("Cast Shadows", &castsShadows))
    {
      setCastsShadows(castsShadows);
    }

    // Only show shadow controls when shadows are enabled
    if (_castsShadows)
    {
      // Shadow resolution dropdown
      const char *resolutionOptions[] = {"256", "512", "1024", "2048", "4096"};
      int currentResIndex = 2; // default to 1024
      uint32 resolutions[] = {256, 512, 1024, 2048, 4096};

      // Find current resolution index
      for (int i = 0; i < 5; i++)
      {
        if (resolutions[i] == _shadowResolution)
        {
          currentResIndex = i;
          break;
        }
      }

      if (ImGui::Combo("Shadow Resolution", &currentResIndex, resolutionOptions, 5))
      {
        setShadowResolution(resolutions[currentResIndex]);
      }

      // Near and far plane controls for point lights
      if (_lightType == LightComponentType::Point)
      {
        float32 nearPlane = _shadowNearPlane;
        if (ImGui::SliderFloat("Shadow Near Plane", &nearPlane, 0.01f, 10.0f))
        {
          setShadowNearPlane(nearPlane);
        }

        float32 farPlane = _shadowFarPlane;
        if (ImGui::SliderFloat("Shadow Far Plane", &farPlane, 1.0f, 500.0f))
        {
          setShadowFarPlane(farPlane);
        }
      }
    }

    // Display culling statistics for point lights
    if (_lightType == LightComponentType::Point && _cullingStats.hasValidStats)
    {
      ImGui::Separator();
      ImGui::Text("Shadow Culling Statistics (Last Frame)");

      ImGui::Text("Total Objects: %u", _cullingStats.totalObjects);
      ImGui::Text("Failed Sphere Culling: %u (%.1f%%)",
                  _cullingStats.objectsFailedSphere,
                  _cullingStats.sphereCullingRatio * 100.0f);
      ImGui::Text("Failed Face Culling: %u (%.1f%%)",
                  _cullingStats.objectsFailedFace,
                  _cullingStats.faceCullingRatio * 100.0f);
      ImGui::Text("Final Rendered: %u", _cullingStats.finalRenderedObjects);

      // Color-coded efficiency indicator
      if (_cullingStats.totalObjects > 0)
      {
        float overallEfficiency = (1.0f - (static_cast<float>(_cullingStats.finalRenderedObjects) / _cullingStats.totalObjects)) * 100.0f;

        if (overallEfficiency > 70.0f)
        {
          ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Culling Efficiency: Excellent (%.1f%%)", overallEfficiency);
        }
        else if (overallEfficiency > 40.0f)
        {
          ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Culling Efficiency: Good (%.1f%%)", overallEfficiency);
        }
        else
        {
          ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Culling Efficiency: Poor (%.1f%%)", overallEfficiency);
        }
      }
    }
  }
}

LightComponent &LightComponent::setColour(const Colour &colour)
{
  _colour = colour;
  _modified = true;
  return *this;
}

LightComponent &LightComponent::setRadius(float32 radius)
{
  _radius = radius;
  _modified = true;
  return *this;
}

LightComponent &LightComponent::setLightType(LightComponentType lightType)
{
  _lightType = lightType;
  _modified = true;
  return *this;
}

LightComponent &LightComponent::setIntensity(float32 intensity)
{
  _intensity = intensity;
  _modified = true;
  return *this;
}

LightComponent &LightComponent::setCastsShadows(bool castsShadows)
{
  _castsShadows = castsShadows;
  _modified = true;
  return *this;
}

LightComponent &LightComponent::setShadowResolution(uint32 resolution)
{
  _shadowResolution = resolution;
  _modified = true;
  return *this;
}

LightComponent &LightComponent::setShadowNearPlane(float32 nearPlane)
{
  _shadowNearPlane = nearPlane;
  _modified = true;
  return *this;
}

LightComponent &LightComponent::setShadowFarPlane(float32 farPlane)
{
  _shadowFarPlane = farPlane;
  _modified = true;
  return *this;
}

void LightComponent::onUpdate(float32 dt)
{
  // Check if transform has changed using direct query
  if (auto transform = getComponentShared<TransformComponent>())
  {
    if (transform->hasChanged() || _modified)
    {
      updateFromTransform(); // Update direction if needed
      recalculateMatrix();
      _modified = false;
      // Note: We don't call transform->clearDirty() here since other components might need to check it too
    }
  }
  else if (_modified)
  {
    // No transform component, but we have local changes
    recalculateMatrix();
    _modified = false;
  }
}

void LightComponent::updateFromTransform()
{
  if (auto transform = getComponentShared<TransformComponent>())
  {
    Quaternion rotation = transform->getRotation();
    Vector3 newDirection = rotation.Rotate(Vector3(0, -1, 0));
    newDirection.Normalize();

    // Only mark as modified if direction actually changed
    if (_direction != newDirection)
    {
      _direction = newDirection;
      _modified = true;
    }
  }
}

void LightComponent::recalculateMatrix()
{
  if (auto transform = getComponentShared<TransformComponent>())
  {
    Vector3 position = transform->getPosition();
    Quaternion rotation = transform->getRotation();

    Matrix4 translation = Matrix4::Translation(position);
    Matrix4 scale = Matrix4::Scaling(Vector3(_radius));
    Matrix4 rotationMatrix = Matrix4::Rotation(rotation);
    _matrix = translation * scale * rotationMatrix;
  }
}

void LightComponent::setCullingStats(const void *cullingResultPtr)
{
  if (_lightType != LightComponentType::Point)
  {
    _cullingStats.reset();
    return;
  }

  // Cast the void pointer back to the correct type
  const PointLightCuller::CullingResult &result = *static_cast<const PointLightCuller::CullingResult *>(cullingResultPtr);

  _cullingStats.totalObjects = result.originalCount;
  _cullingStats.objectsFailedSphere = result.originalCount - result.sphereCulledCount;

  // For face culling, we need to calculate how many objects failed face culling
  // by deduplicating the face-culled objects and comparing to sphere-culled count
  if (result.sphereCulledCount > 0)
  {
    // Combine all face-culled objects and deduplicate
    std::vector<std::shared_ptr<DrawableComponent>> allFaceCulledObjects;
    for (int face = 0; face < 6; ++face)
    {
      const auto &faceObjects = result.faceCulled[face];
      allFaceCulledObjects.insert(allFaceCulledObjects.end(), faceObjects.begin(), faceObjects.end());
    }

    // Remove duplicates since objects can appear in multiple faces
    std::sort(allFaceCulledObjects.begin(), allFaceCulledObjects.end());
    allFaceCulledObjects.erase(std::unique(allFaceCulledObjects.begin(), allFaceCulledObjects.end()), allFaceCulledObjects.end());

    _cullingStats.finalRenderedObjects = static_cast<uint32>(allFaceCulledObjects.size());
    _cullingStats.objectsFailedFace = result.sphereCulledCount - _cullingStats.finalRenderedObjects;
  }
  else
  {
    _cullingStats.finalRenderedObjects = 0;
    _cullingStats.objectsFailedFace = 0;
  }

  // Calculate ratios
  if (_cullingStats.totalObjects > 0)
  {
    _cullingStats.sphereCullingRatio = static_cast<float>(_cullingStats.objectsFailedSphere) / _cullingStats.totalObjects;

    if (result.sphereCulledCount > 0)
    {
      _cullingStats.faceCullingRatio = static_cast<float>(_cullingStats.objectsFailedFace) / result.sphereCulledCount;
    }
    else
    {
      _cullingStats.faceCullingRatio = 0.0f;
    }
  }
  else
  {
    _cullingStats.sphereCullingRatio = 0.0f;
    _cullingStats.faceCullingRatio = 0.0f;
  }

  _cullingStats.hasValidStats = true;
}
