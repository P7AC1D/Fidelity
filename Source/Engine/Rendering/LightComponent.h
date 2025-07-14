#pragma once
#include "../Core/IComponent.h"
#include "../Core/IUpdatableComponent.h"
#include "../Core/ComponentTypeId.h"
#include "../Core/ComponentDependency.h"
#include "../Core/Maths.h"
#include <memory>

// Forward declarations
class TransformComponent;

enum class LightComponentType
{
    Point,
    Directional,
    Spot,
    Area
};

/// Modern Light component that implements IComponent interface.
/// Handles all light types with shadow support and proper transform integration.
/// Depends on TransformComponent for spatial information.
class LightComponent : public IComponent, public IComponentDependency, public IUpdatableComponent
{
public:
    LightComponent();
    LightComponent(LightComponentType lightType, const Colour& colour = Colour::White, float32 intensity = 1000.0f);

    // IComponent interface
    void initialize() override;
    void activate() override;
    void deactivate() override;
    ComponentTypeId getTypeId() const override { return getComponentTypeId<LightComponent>(); }
    void drawInspector() override;

    // Static type information
    static ComponentTypeId GetTypeId() { return getComponentTypeId<LightComponent>(); }

    // Light configuration methods (fluent interface)
    LightComponent& setColour(const Colour& colour);
    LightComponent& setRadius(float32 radius);
    LightComponent& setLightType(LightComponentType lightType);
    LightComponent& setIntensity(float32 intensity);

    // Shadow system support
    LightComponent& setCastsShadows(bool castsShadows);
    LightComponent& setShadowResolution(uint32 resolution);
    LightComponent& setShadowNearPlane(float32 nearPlane);
    LightComponent& setShadowFarPlane(float32 farPlane);

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

    // IComponentDependency interface
    std::vector<ComponentTypeId> getDependencies() const override;
    void onDependenciesResolved(GameObject& gameObject) override;

    // Transform integration
    void setTransformComponent(std::weak_ptr<TransformComponent> transform);
    std::weak_ptr<TransformComponent> getTransformComponent() const { return _transformComponent; }

    // Update method for recalculating matrices and positions
    void update(float32 dt) override;

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

    // State tracking
    bool _modified;

    // Transform dependency
    std::weak_ptr<TransformComponent> _transformComponent;
};
