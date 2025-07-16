#include "CameraComponent.h"
#include "../Core/TransformComponent.h"
#include "../Core/GameObject.h"
#include "../Core/ComponentBase.inl"
#include "../UI/ImGui/imgui.h"

CameraComponent::CameraComponent()
{
  updateProjection();
}

CameraComponent::~CameraComponent()
{
}

void CameraComponent::onInitialize()
{
  // CRITICAL FIX: Ensure we mark view as dirty to force recalculation with proper transform
  // This ensures proper dependency resolution with TransformComponent
  _viewDirty = true;
  _frustumDirty = true;

  // Initialize camera projection
  updateProjection();
}

void CameraComponent::onActivate()
{
  // Component is now active - could register with rendering system here
}

void CameraComponent::onDeactivate()
{
  // Component is now inactive - could unregister from rendering system here
}

void CameraComponent::onUpdate(float32 dt)
{
  if (auto transform = getComponentShared<TransformComponent>())
  {
    if (transform->hasChanged())
    {
      _viewDirty = true;
      _frustumDirty = true;
    }
  }
}

ComponentTypeId CameraComponent::getTypeId() const
{
  return GetTypeId();
}

void CameraComponent::drawInspector()
{
  if (ImGui::CollapsingHeader("Camera"))
  {
    // Camera properties
    ImGui::Text("Camera Properties");

    // Resolution
    int width = _width;
    int height = _height;
    if (ImGui::InputInt("Width", &width) && width > 0)
    {
      setWidth(width);
    }
    if (ImGui::InputInt("Height", &height) && height > 0)
    {
      setHeight(height);
    }

    // Field of view
    float fovDegrees = _fov.InDegrees();
    if (ImGui::SliderFloat("FOV", &fovDegrees, 10.0f, 120.0f))
    {
      setFov(Degree(fovDegrees));
    }

    // Near/Far planes
    float near = _near;
    float far = _far;
    if (ImGui::InputFloat("Near Plane", &near) && near > 0.0f)
    {
      setNear(near);
    }
    if (ImGui::InputFloat("Far Plane", &far) && far > near)
    {
      setFar(far);
    }

    // Calculated values
    ImGui::Separator();
    ImGui::Text("Calculated Values");
    ImGui::Text("Aspect Ratio: %.3f", getAspectRatio());

    // World space information
    if (auto transform = getComponentShared<TransformComponent>())
    {
      Vector3 worldPos = getWorldPosition();
      Vector3 worldForward = getWorldForward();

      ImGui::Text("World Position: (%.2f, %.2f, %.2f)", worldPos.X, worldPos.Y, worldPos.Z);
      ImGui::Text("World Forward: (%.2f, %.2f, %.2f)", worldForward.X, worldForward.Y, worldForward.Z);
    }
    else
    {
      ImGui::Text("No Transform Component");
    }
  }
}

CameraComponent &CameraComponent::setPerspective(const Degree &fovY, int32 width, int32 height, float32 nearClip, float32 farClip)
{
  _fov = fovY;
  _width = width;
  _height = height;
  _near = nearClip;
  _far = farClip;

  updateProjection();
  return *this;
}

CameraComponent &CameraComponent::setHeight(int32 height)
{
  if (_height != height)
  {
    _height = height;
    updateProjection();
  }
  return *this;
}

CameraComponent &CameraComponent::setWidth(int32 width)
{
  if (_width != width)
  {
    _width = width;
    updateProjection();
  }
  return *this;
}

CameraComponent &CameraComponent::setFov(const Degree &fov)
{
  if (_fov != fov)
  {
    _fov = fov;
    updateProjection();
  }
  return *this;
}

CameraComponent &CameraComponent::setNear(float32 near)
{
  if (_near != near)
  {
    _near = near;
    updateProjection();
  }
  return *this;
}

CameraComponent &CameraComponent::setFar(float32 far)
{
  if (_far != far)
  {
    _far = far;
    updateProjection();
  }
  return *this;
}

Matrix4 CameraComponent::getView() const
{
  if (_viewDirty)
  {
    updateView();
  }

  // DEFENSIVE CHECK: If view matrix is still identity after update attempt,
  // force a recalculation on next access to handle dependency timing issues
  if (_view == Matrix4::Identity)
  {
    _viewDirty = true;
  }

  return _view;
}

const Frustrum &CameraComponent::getFrustum() const
{
  if (_frustumDirty)
  {
    updateFrustum();
  }
  return _frustum;
}

bool CameraComponent::contains(const Aabb &aabb, const Matrix4 &transform) const
{
  // Get the frustum for proper culling
  const Frustrum &frustum = getFrustum();

  // Use the new Matrix4 overload to avoid temporary TransformComponent creation
  return frustum.contains(aabb, transform);
}

float32 CameraComponent::distanceFrom(const Vector3 &position) const
{
  Vector3 cameraPos = getWorldPosition();
  return (position - cameraPos).Length();
}

Vector3 CameraComponent::getWorldPosition() const
{
  if (auto transform = getComponentShared<TransformComponent>())
  {
    return transform->getPosition();
  }

  // CRITICAL FIX: Try direct access if shared pointer approach fails
  if (_gameObject)
  {
    if (auto *directTransform = _gameObject->tryGetComponent<TransformComponent>())
    {
      return directTransform->getPosition();
    }
  }

  return Vector3::Zero;
}

Vector3 CameraComponent::getWorldForward() const
{
  auto transform = getComponentShared<TransformComponent>();
  if (!transform)
  {
    // CRITICAL FIX: If we can't get the TransformComponent, try direct access
    // This handles timing issues in component dependency resolution
    if (_gameObject)
    {
      if (auto *directTransform = _gameObject->tryGetComponent<TransformComponent>())
      {
        Vector3 forward(0.0f, 0.0f, -1.0f);
        Matrix4 rotationMatrix = Matrix4::Rotation(directTransform->getRotation());
        Vector3 result = rotationMatrix * forward;
        result.Normalize();
        return result;
      }
    }
    return Vector3(0.0f, 0.0f, -1.0f); // Default forward
  }

  // Transform forward vector (0, 0, -1) by rotation
  Vector3 forward(0.0f, 0.0f, -1.0f);
  Matrix4 rotationMatrix = Matrix4::Rotation(transform->getRotation());
  Vector3 result = rotationMatrix * forward;
  result.Normalize();
  return result;
}

Vector3 CameraComponent::getWorldUp() const
{
  auto transform = getComponentShared<TransformComponent>();
  if (!transform)
  {
    // CRITICAL FIX: If we can't get the TransformComponent, try direct access
    if (_gameObject)
    {
      if (auto *directTransform = _gameObject->tryGetComponent<TransformComponent>())
      {
        Vector3 up(0.0f, 1.0f, 0.0f);
        Matrix4 rotationMatrix = Matrix4::Rotation(directTransform->getRotation());
        Vector3 result = rotationMatrix * up;
        result.Normalize();
        return result;
      }
    }
    return Vector3(0.0f, 1.0f, 0.0f); // Default up
  }

  // Transform up vector (0, 1, 0) by rotation
  Vector3 up(0.0f, 1.0f, 0.0f);
  Matrix4 rotationMatrix = Matrix4::Rotation(transform->getRotation());
  Vector3 result = rotationMatrix * up;
  result.Normalize();
  return result;
}

Vector3 CameraComponent::getWorldRight() const
{
  auto transform = getComponentShared<TransformComponent>();
  if (!transform)
  {
    return Vector3(1.0f, 0.0f, 0.0f); // Default right
  }

  // Transform right vector (1, 0, 0) by rotation
  Vector3 right(1.0f, 0.0f, 0.0f);
  Matrix4 rotationMatrix = Matrix4::Rotation(transform->getRotation());
  Vector3 result = rotationMatrix * right;
  result.Normalize();
  return result;
}

void CameraComponent::updateView() const
{
  auto transform = getComponentShared<TransformComponent>();
  if (!transform)
  {
    // CRITICAL FIX: Try direct access if shared pointer approach fails
    if (_gameObject)
    {
      if (auto *directTransform = _gameObject->tryGetComponent<TransformComponent>())
      {
        Vector3 eye = directTransform->getPosition();
        Vector3 forward = getWorldForward();
        Vector3 up = getWorldUp();
        Vector3 center = eye + forward;

        _view = Matrix4::LookAt(eye, center, up);
        _viewDirty = false;
        return;
      }
    }

    // Final fallback: create a proper default view matrix for an uninitialized camera
    // This prevents the overly permissive fallback frustum behavior
    _view = Matrix4::LookAt(Vector3(0, 0, 5), Vector3(0, 0, 0), Vector3(0, 1, 0));
    _viewDirty = false;
    return;
  }

  Vector3 eye = transform->getPosition();
  Vector3 forward = getWorldForward();
  Vector3 up = getWorldUp();
  Vector3 center = eye + forward;

  _view = Matrix4::LookAt(eye, center, up);
  _viewDirty = false;
}

void CameraComponent::updateProjection()
{
  float32 aspectRatio = getAspectRatio();
  _proj = Matrix4::Perspective(_fov, aspectRatio, _near, _far);
  _projDirty = false;
  _frustumDirty = true;
}

void CameraComponent::updateFrustum() const
{
  if (_frustumDirty)
  {
    _frustum = Frustrum(*this);
    _frustumDirty = false;
  }
}
