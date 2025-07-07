#include "CameraComponent.h"
#include "../Core/TransformComponent.h"
#include "../Core/GameObject.h"
#include "../UI/ImGui/imgui.h"

CameraComponent::CameraComponent()
{
    updateProjection();
}

void CameraComponent::initialize()
{
    // Initialize camera projection
    updateProjection();
}

void CameraComponent::activate()
{
    // Component is now active - could register with rendering system here
}

void CameraComponent::deactivate()
{
    // Component is now inactive - could unregister from rendering system here
}

ComponentTypeId CameraComponent::getTypeId() const
{
    return GetTypeId();
}

std::vector<ComponentTypeId> CameraComponent::getDependencies() const
{
    return { getComponentTypeId<TransformComponent>() };
}

void CameraComponent::onDependenciesResolved(GameObject& gameObject)
{
    // Get the TransformComponent from the GameObject
    if (auto* transform = gameObject.tryGetComponent<TransformComponent>())
    {
        // Store raw pointer since GameObject guarantees component lifetime
        _transformRawPtr = transform;
        _viewDirty = true;
        _frustumDirty = true;
    }
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
        if (const TransformComponent* transform = getTransformComponent())
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

CameraComponent& CameraComponent::setPerspective(const Degree& fovY, int32 width, int32 height, float32 nearClip, float32 farClip)
{
    _fov = fovY;
    _width = width;
    _height = height;
    _near = nearClip;
    _far = farClip;
    
    updateProjection();
    return *this;
}

CameraComponent& CameraComponent::setHeight(int32 height)
{
    if (_height != height)
    {
        _height = height;
        updateProjection();
    }
    return *this;
}

CameraComponent& CameraComponent::setWidth(int32 width)
{
    if (_width != width)
    {
        _width = width;
        updateProjection();
    }
    return *this;
}

CameraComponent& CameraComponent::setFov(const Degree& fov)
{
    if (_fov != fov)
    {
        _fov = fov;
        updateProjection();
    }
    return *this;
}

CameraComponent& CameraComponent::setNear(float32 near)
{
    if (_near != near)
    {
        _near = near;
        updateProjection();
    }
    return *this;
}

CameraComponent& CameraComponent::setFar(float32 far)
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
    return _view;
}

const Frustrum& CameraComponent::getFrustum() const
{
    if (_frustumDirty)
    {
        updateFrustum();
    }
    return _frustum;
}

bool CameraComponent::contains(const Aabb& aabb, const Matrix4& transform) const
{
    // For now, use simple distance-based culling
    // TODO: Implement proper AABB transformation and frustum testing
    Vector3 aabbCenter = (aabb.getMin() + aabb.getMax()) * 0.5f;
    Vector3 worldCenter = transform * aabbCenter;
    
    return distanceFrom(worldCenter) < _far;
}

float32 CameraComponent::distanceFrom(const Vector3& position) const
{
    Vector3 cameraPos = getWorldPosition();
    return (position - cameraPos).Length();
}

void CameraComponent::setTransformComponent(std::weak_ptr<TransformComponent> transform)
{
    _transformComponent = transform;
    _viewDirty = true;
    _frustumDirty = true;
}

void CameraComponent::setTransformComponentForTesting(TransformComponent* transform)
{
    _transformRawPtr = transform;
    _viewDirty = true;
    _frustumDirty = true;
}

Vector3 CameraComponent::getWorldPosition() const
{
    if (const TransformComponent* transform = getTransformComponent())
    {
        return transform->getPosition();
    }
    return Vector3::Zero;
}

Vector3 CameraComponent::getWorldForward() const
{
    const TransformComponent* transform = getTransformComponent();
    if (!transform)
    {
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
    const TransformComponent* transform = getTransformComponent();
    if (!transform)
    {
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
    const TransformComponent* transform = getTransformComponent();
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
    const TransformComponent* transform = getTransformComponent();
    if (!transform)
    {
        _view = Matrix4::Identity;
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
    if (_viewDirty)
    {
        updateView();
    }
    
    // Build frustum from this camera
    _frustum = Frustrum(*this);
    _frustumDirty = false;
}

const TransformComponent* CameraComponent::getTransformComponent() const
{
    return _transformRawPtr;
}
