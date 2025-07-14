#pragma once

#include <memory>
#include "../Core/IComponent.h"
#include "../Core/IUpdatableComponent.h"
#include "../Core/ComponentTypeId.h"
#include "../Core/ComponentDependency.h"
#include "../Core/Maths.h"
#include "../Core/TransformComponent.h"
#include "../Core/Types.hpp"

/// Camera component that implements IComponent interface.
/// Handles camera projection, view matrix calculation, and frustum culling.
/// Depends on TransformComponent for spatial information.
class CameraComponent : public IComponent, public IComponentDependency, public IUpdatableComponent
{
public:
    CameraComponent();
    ~CameraComponent();

    // IComponent interface
    void initialize() override;
    void activate() override;
    void deactivate() override;
    ComponentTypeId getTypeId() const override;
    void drawInspector() override;

    // Static type information
    static ComponentTypeId GetTypeId() { return getComponentTypeId<CameraComponent>(); }

    // Camera configuration
    CameraComponent& setPerspective(const Degree& fovY, int32 width, int32 height, float32 nearClip, float32 farClip);
    CameraComponent& setHeight(int32 height);
    CameraComponent& setWidth(int32 width);
    CameraComponent& setFov(const Degree& fov);
    CameraComponent& setNear(float32 near);
    CameraComponent& setFar(float32 far);

    // Camera properties
    Matrix4 getView() const;
    Matrix4 getProj() const { return _proj; }
    int32 getWidth() const { return _width; }
    int32 getHeight() const { return _height; }
    Radian getFov() const { return _fov; }
    float32 getNear() const { return _near; }
    float32 getFar() const { return _far; }
    float32 getAspectRatio() const { return _width / static_cast<float32>(_height); }

    // Frustum and culling
    const Frustrum& getFrustum() const;
    bool contains(const Aabb& aabb, const Matrix4& transform) const;
    float32 distanceFrom(const Vector3& position) const;

    // IComponentDependency interface
    std::vector<ComponentTypeId> getDependencies() const override;
    void onDependenciesResolved(GameObject& gameObject) override;

    // Transform integration
    void setTransformComponent(std::weak_ptr<TransformComponent> transform);
    std::weak_ptr<TransformComponent> getTransformComponentWeak() const { return _transformComponent; }

    // World space properties
    Vector3 getWorldPosition() const;
    Vector3 getWorldForward() const;
    Vector3 getWorldUp() const;
    Vector3 getWorldRight() const;

    // Change tracking
    bool hasChanged() const { return _viewDirty || _projDirty; }
    void markDirty() { _viewDirty = _projDirty = _frustumDirty = true; }

    // IUpdatableComponent interface
    void update(float32 dt) override;

private:
    // Camera parameters
    int32 _width = 1920;
    int32 _height = 1080;
    Radian _fov = Degree(60.0f);
    float32 _near = 0.1f;
    float32 _far = 1000.0f;

    // Cached matrices
    mutable Matrix4 _view = Matrix4::Identity;
    Matrix4 _proj = Matrix4::Identity;
    mutable Frustrum _frustum;

    // Dirty flags for caching
    mutable bool _viewDirty = true;
    bool _projDirty = true;
    mutable bool _frustumDirty = true;

    // Transform dependency
    std::weak_ptr<TransformComponent> _transformComponent;
    TransformComponent::CallbackId _transformObserverId = 0;

    // Helper methods
    void updateView() const;
    void updateProjection();
    void updateFrustum() const;
    
    const TransformComponent* getTransformComponent() const;
};
