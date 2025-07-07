#include "LightComponent.h"
#include "../Core/TransformComponent.h"
#include "../Core/GameObject.h"
#include "../UI/ImGui/imgui.h"

LightComponent::LightComponent()
    : _colour(Colour::White)
    , _radius(10.0f)
    , _lightType(LightComponentType::Point)
    , _matrix(Matrix4::Identity)
    , _direction(Vector3::Identity)
    , _intensity(1000.0f)
    , _castsShadows(false)
    , _shadowResolution(1024)
    , _shadowNearPlane(0.1f)
    , _shadowFarPlane(100.0f)
    , _modified(true)
{
}

LightComponent::LightComponent(LightComponentType lightType, const Colour& colour, float32 intensity)
    : LightComponent() // Delegate to default constructor
{
    _lightType = lightType;
    _colour = colour;
    _intensity = intensity;
}

void LightComponent::initialize()
{
    // Component is ready for use
    _modified = true;
}

void LightComponent::activate()
{
    // Called when component becomes active
    updateFromTransform();
}

void LightComponent::deactivate()
{
    // Called when component becomes inactive
}

std::vector<ComponentTypeId> LightComponent::getDependencies() const
{
    return { getComponentTypeId<TransformComponent>() };
}

void LightComponent::onDependenciesResolved(GameObject& gameObject)
{
    // Get the TransformComponent from the GameObject
    if (auto* transform = gameObject.tryGetComponent<TransformComponent>())
    {
        // Create a shared_ptr that doesn't own the object (since GameObject owns it)
        auto transformPtr = std::shared_ptr<TransformComponent>(transform, [](TransformComponent*){});
        // Convert to weak_ptr for the light
        std::weak_ptr<TransformComponent> weakPtr = transformPtr;
        setTransformComponent(weakPtr);
    }
}

Vector3 LightComponent::getPosition() const
{
    if (auto transform = _transformComponent.lock())
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
        const char* lightTypeNames[] = { "Point", "Directional", "Spot", "Area" };
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
            const char* resolutionOptions[] = { "256", "512", "1024", "2048", "4096" };
            int currentResIndex = 2; // default to 1024
            uint32 resolutions[] = { 256, 512, 1024, 2048, 4096 };

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

        // Display current position and direction (read-only)
        ImGui::Separator();
        ImGui::Text("Transform Info (Read-Only)");
        Vector3 position = getPosition();
        ImGui::Text("Position: %.2f, %.2f, %.2f", position.X, position.Y, position.Z);
        ImGui::Text("Direction: %.2f, %.2f, %.2f", _direction.X, _direction.Y, _direction.Z);
    }
}

LightComponent& LightComponent::setColour(const Colour& colour)
{
    _colour = colour;
    _modified = true;
    return *this;
}

LightComponent& LightComponent::setRadius(float32 radius)
{
    _radius = radius;
    _modified = true;
    return *this;
}

LightComponent& LightComponent::setLightType(LightComponentType lightType)
{
    _lightType = lightType;
    _modified = true;
    return *this;
}

LightComponent& LightComponent::setIntensity(float32 intensity)
{
    _intensity = intensity;
    _modified = true;
    return *this;
}

LightComponent& LightComponent::setCastsShadows(bool castsShadows)
{
    _castsShadows = castsShadows;
    _modified = true;
    return *this;
}

LightComponent& LightComponent::setShadowResolution(uint32 resolution)
{
    _shadowResolution = resolution;
    _modified = true;
    return *this;
}

LightComponent& LightComponent::setShadowNearPlane(float32 nearPlane)
{
    _shadowNearPlane = nearPlane;
    _modified = true;
    return *this;
}

LightComponent& LightComponent::setShadowFarPlane(float32 farPlane)
{
    _shadowFarPlane = farPlane;
    _modified = true;
    return *this;
}

void LightComponent::setTransformComponent(std::weak_ptr<TransformComponent> transform)
{
    _transformComponent = transform;
    updateFromTransform();
}

void LightComponent::update(float32 dt)
{
    updateFromTransform();
    
    if (_modified)
    {
        recalculateMatrix();
        _modified = false;
    }
}

void LightComponent::updateFromTransform()
{
    if (auto transform = _transformComponent.lock())
    {
        Quaternion rotation = transform->getRotation();
        _direction = rotation.Rotate(Vector3(0, -1, 0));
        _direction.Normalize();
        _modified = true;
    }
}

void LightComponent::recalculateMatrix()
{
    if (auto transform = _transformComponent.lock())
    {
        Vector3 position = transform->getPosition();
        Quaternion rotation = transform->getRotation();
        
        Matrix4 translation = Matrix4::Translation(position);
        Matrix4 scale = Matrix4::Scaling(Vector3(_radius));
        Matrix4 rotationMatrix = Matrix4::Rotation(rotation);
        _matrix = translation * scale * rotationMatrix;
    }
}
