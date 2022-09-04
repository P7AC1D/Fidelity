#pragma once
#include <memory>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "../Rendering/Drawable.h"
#include "../Rendering/Camera.h"
#include "../Rendering/Light.h"
#include "Component.h"
#include "Maths.h"
#include "Types.hpp"

class GameObject;
class DeferredRenderer;
class RenderDevice;

class Scene
{
public:
  bool init(const Vector2I &windowDims, std::shared_ptr<RenderDevice> renderDevice);

  template <typename T, typename... Args>
  T &createComponent(Args... args);

  GameObject &createGameObject(const std::string &name);

  void update(float64 dt);
  void drawFrame() const;

  Camera &getCamera() { return _camera; }

  // TODO Remove this and better abstract dependenciexc
  std::shared_ptr<RenderDevice> getRenderDevice() { return _renderDevice; }

private:
  struct DrawableSortMap
  {
    float32 DistanceToCamera;
    std::shared_ptr<Component> ComponentPtr;

    DrawableSortMap(float32 distance, std::shared_ptr<Component> component) : DistanceToCamera(distance),
                                                                              ComponentPtr(component)
    {
    }
  };

  std::unordered_map<ComponentType, std::vector<std::shared_ptr<Component>>> _components;
  std::vector<std::shared_ptr<GameObject>> _gameObject;
  Camera _camera;

  std::shared_ptr<DeferredRenderer> _deferredRenderer;
  std::shared_ptr<RenderDevice> _renderDevice;
};

template <typename T, typename... Args>
T &Scene::createComponent(Args... args)
{
  static_assert(std::is_base_of<Component, T>::value, "Type is not derived from Component.");

  auto component = std::make_shared<T>(args...);
  _components[component->getType()].push_back(component);
  return *component.get();
}