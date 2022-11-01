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
class InputHandler;
class Light;
class Renderer;
class RenderDevice;
class SceneGraph;

class Scene
{
public:
  Scene(const std::shared_ptr<InputHandler> &inputHandler);
  virtual ~Scene();
  bool init(const Vector2I &windowDims, std::shared_ptr<RenderDevice> renderDevice);

  void setMouseCoordinates(const Vector2I &coords) { _mouseCoordinates = coords; }

  void update(float32 dt);
  void drawFrame();
  void drawDebugUi();

  void setMainCamera(const std::shared_ptr<Camera> &camera) { _mainCamera = camera; }
  const std::shared_ptr<GameObject> &getRootGameObject() const { return _rootGameObject; }

  // TODO Remove this and better abstract dependenciexc
  std::shared_ptr<RenderDevice> getRenderDevice() { return _renderDevice; }

private:
  void drawSceneGraphEditor(const std::shared_ptr<GameObject> &gameObject);
  void drawGameObjectInspector(const std::shared_ptr<GameObject> &gameObject);
  void setAabbDrawOnGameObject(const std::shared_ptr<GameObject> &gameObject, bool enableAabbDraw);

  void drawGameObject(const std::shared_ptr<GameObject> &gameObject,
                      std::vector<std::shared_ptr<Drawable>> &opaqueDrawables,
                      std::vector<std::shared_ptr<Drawable>> &transparentDrawables,
                      std::vector<std::shared_ptr<Drawable>> &allDrawables,
                      std::vector<std::shared_ptr<Drawable>> &aabbDrawables,
                      std::vector<std::shared_ptr<GameObject>> &mousePickedObjects,
                      std::vector<std::shared_ptr<Light>> &lights,
                      const Ray &mouseCoordsProjectedRay,
                      const std::shared_ptr<Camera> &camera) const;

  bool _objectAddedToScene;
  uint64 _scenePrepDuration;
  Vector2I _mouseCoordinates;
  Vector2I _windowDims;

  std::shared_ptr<Renderer> _renderer;
  std::shared_ptr<RenderDevice> _renderDevice;
  std::shared_ptr<InputHandler> _inputHandler;
  std::shared_ptr<GameObject> _rootGameObject;
  std::shared_ptr<Camera> _mainCamera;
};