#pragma once
#include <memory>
#include <type_traits>
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
  const std::vector<Drawable> &getDrawables() const { return _drawables; }

  // TODO Remove this and better abstract dependenciexc
  std::shared_ptr<RenderDevice> getRenderDevice() { return _renderDevice; }

private:
  struct DrawableSortMap
  {
    float32 DistanceToCamera;
    uint64 Index;

    DrawableSortMap(float32 distance, uint64 index)
    {
      DistanceToCamera = distance;
      Index = index;
    }
  };

  std::vector<GameObject> _gameObject;
  std::vector<Drawable> _drawables;
  std::vector<Light> _lights;
  std::vector<Component> _generalComponents;
  Camera _camera;

  std::shared_ptr<DeferredRenderer> _deferredRenderer;
  std::shared_ptr<RenderDevice> _renderDevice;
};

template <typename T, typename... Args>
T &Scene::createComponent(Args... args)
{
  static_assert(std::is_base_of<Component, T>::value, "Type is not derived from Component.");

  if (std::is_base_of<Drawable, T>::value)
  {
    _drawables.emplace_back(args...);
    return dynamic_cast<T &>(_drawables[_drawables.size() - 1]);
  }

  if (std::is_base_of<Light, T>::value)
  {
    _lights.emplace_back(args...);
    return dynamic_cast<T &>(_lights[_lights.size() - 1]);
  }

  _generalComponents.emplace_back(args...);
  return dynamic_cast<T &>(_generalComponents[_generalComponents.size() - 1]);
}