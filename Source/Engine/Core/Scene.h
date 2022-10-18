#pragma once
#include <functional>
#include <map>
#include <memory>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "Component.h"
#include "Maths.h"
#include "Types.hpp"

class Camera;
class Drawable;
class GameObject;
class Renderer;
class RenderDevice;
class SceneGraph;

class Scene
{
public:
  Scene();
  virtual ~Scene();
  bool init(const Vector2I &windowDims, std::shared_ptr<RenderDevice> renderDevice);

  template <typename T>
  T &createComponent();
  template <typename T, typename... Args>
  T &createComponent(Args... args);

  GameObject &createGameObject(const std::string &name);

  void addChildToNode(GameObject &parent, GameObject &child);

  void setMouseCoordinates(const Vector2I &coords) { _mouseCoordinates = coords; }

  void update(float64 dt);
  void drawFrame();
  void drawDebugUi();

  GameObject &getRoot() { return _gameObjects[0]; }

  // TODO Remove this and better abstract dependenciexc
  std::shared_ptr<RenderDevice> getRenderDevice() { return _renderDevice; }

private:
  void drawSceneGraphUi(int64 nodeIndex);
  void drawGameObjectInspector(int64 selectedGameObjectIndex);
  void setAabbDrawOnGameObject(int64 gameObjectIndex, bool enableAabbDraw);

  struct DrawableSortMap
  {
    float32 DistanceToCamera;
    std::shared_ptr<Drawable> ComponentPtr;

    DrawableSortMap(float32 distance, std::shared_ptr<Drawable> component) : DistanceToCamera(distance),
                                                                             ComponentPtr(component)
    {
    }
  };

  bool _objectAddedToScene;
  uint64 _scenePrepDuration;
  Vector2I _mouseCoordinates;
  Vector2I _windowDims;

  std::unique_ptr<SceneGraph> _sceneGraph;
  // TODO: Remove pointers here so that its a true data-driven design
  std::unordered_map<ComponentType, std::vector<std::shared_ptr<Component>>> _components;
  std::map<uint64, GameObject> _gameObjects;

  std::shared_ptr<Renderer> _renderer;
  std::shared_ptr<RenderDevice> _renderDevice;
};

template <typename T>
T &Scene::createComponent()
{
  static_assert(std::is_base_of<Component, T>::value, "Type is not derived from Component.");

  auto component = std::shared_ptr<T>(new T());
  _components[component->getType()].push_back(component);

  auto componentOfT = std::static_pointer_cast<T>(_components[component->getType()].back());
  return *(componentOfT.get());
}

template <typename T, typename... Args>
T &Scene::createComponent(Args... args)
{
  static_assert(std::is_base_of<Component, T>::value, "Type is not derived from Component.");

  auto component = std::shared_ptr<T>(new T(args...));
  _components[component->getType()].push_back(component);

  auto componentOfT = std::static_pointer_cast<T>(_components[component->getType()].back());
  return *(componentOfT.get());
}