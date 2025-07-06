#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <functional>

#include "GameObject.h"
#include "ComponentManager.h"
#include "Types.hpp"
#include "Maths.h"

class DrawableComponent;
class InputHandler;
class Renderer;
class RenderDevice;
class LightComponent;

/// Scene implementation that works with GameObject and modern components.
class Scene
{
public:
    Scene(const std::shared_ptr<InputHandler>& inputHandler);
    virtual ~Scene();

    /// Initialize the scene with window dimensions and render device
    bool init(const Vector2I& windowDims, std::shared_ptr<RenderDevice> renderDevice);

    /// Create a new GameObject with modern component system
    GameObject& createGameObject(const std::string& name);

    /// Add a child GameObject to a parent
    void addChild(GameObject& parent, std::unique_ptr<GameObject> child);

    /// Update all GameObjects and their components
    void update(float32 dt);

    /// Render the scene
    void drawFrame();

    /// Draw debug UI
    void drawDebugUi();

    /// Set mouse coordinates for object picking
    void setMouseCoordinates(const Vector2I& coords) { _mouseCoordinates = coords; }

    /// Get the root GameObject
    GameObject& getRoot() { return *_rootObject; }

    /// Get render device (for compatibility)
    std::shared_ptr<RenderDevice> getRenderDevice() { return _renderDevice; }

    /// Get all cameras in the scene
    std::vector<CameraComponent*> getCameras();

    /// Get all lights in the scene
    std::vector<LightComponent*> getLights();

    /// Get all drawable objects in the scene
    std::vector<DrawableComponent*> getDrawables();

    /// Get the main camera (first camera found)
    CameraComponent* getMainCamera();

private:
    /// Collect all components of a specific type from all GameObjects
    template<typename T>
    std::vector<T*> collectComponents();

    /// Recursively collect components from a GameObject and its children
    template<typename T>
    void collectComponentsRecursive(GameObject& gameObject, std::vector<T*>& components);

    /// Perform object picking with the camera
    void performObjectPicker(const CameraComponent& camera);

    /// Draw scene graph UI
    void drawSceneGraphUi(GameObject& gameObject, int depth = 0);

    /// Draw GameObject inspector
    void drawGameObjectInspector(GameObject* selectedGameObject);

    /// Sort drawables by distance to camera for transparency
    struct DrawableDistance
    {
        float32 distance;
        DrawableComponent* drawable;
        
        DrawableDistance(float32 dist, DrawableComponent* comp) 
            : distance(dist), drawable(comp) {}
    };

    std::vector<DrawableDistance> sortDrawablesByDistance(const CameraComponent& camera);

private:
    // Core systems
    std::unique_ptr<ComponentManager> _componentManager;
    std::shared_ptr<Renderer> _renderer;
    std::shared_ptr<RenderDevice> _renderDevice;
    std::shared_ptr<InputHandler> _inputHandler;

    // Scene data
    std::unique_ptr<GameObject> _rootObject;
    std::vector<std::unique_ptr<GameObject>> _gameObjects;
    uint64 _nextGameObjectId = 1;

    // UI state
    Vector2I _mouseCoordinates;
    Vector2I _windowDims;
    GameObject* _selectedGameObject = nullptr;
    bool _objectAddedToScene = false;
    uint64 _scenePrepDuration = 0;
};

// Template implementations
template<typename T>
std::vector<T*> Scene::collectComponents()
{
    std::vector<T*> components;
    if (_rootObject)
    {
        collectComponentsRecursive<T>(*_rootObject, components);
    }
    return components;
}

template<typename T>
void Scene::collectComponentsRecursive(GameObject& gameObject, std::vector<T*>& components)
{
    // Check if this GameObject has the component
    if (auto* component = gameObject.tryGetComponent<T>())
    {
        components.push_back(component);
    }

    // Recursively check children
    for (const auto& child : gameObject.getChildren())
    {
        collectComponentsRecursive<T>(*child, components);
    }
}
