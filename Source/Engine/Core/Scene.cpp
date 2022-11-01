#include "Scene.h"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <list>
#include <set>
#include <string>
#include <vector>

#include "../UI/ImGui/imgui.h"
#include "../Utility/ModelLoader.hpp"
#include "../Rendering/Camera.h"
#include "../Rendering/Renderer.h"
#include "../Rendering/Drawable.h"
#include "../Rendering/Material.h"
#include "../Rendering/Light.h"
#include "../RenderApi/RenderDevice.hpp"
#include "GameObject.h"
#include "InputHandler.h"

static std::shared_ptr<GameObject> SELECTED_GAME_OBJECT = nullptr;

/// @brief Builds a projected ray in world space from the a set of mouse coordinates in screen space.
/// @param mouseCoords The current mouse coordinates in screen space.
/// @param windowDims The current window's dimensions.
/// @param camera The current active camera.
/// @return The projected ray in world space.
Ray buildRayFromMouseCoords(const Vector2I &mouseCoords, const Vector2I &windowDims, const Camera &camera)
{
  float32 x = (static_cast<float32>(mouseCoords.X) / windowDims.X) * 2.f - 1.f;
  float32 y = 1.0f - (static_cast<float32>(mouseCoords.Y) / windowDims.Y) * 2.f;

  // We want our ray's z to point forwards - this is usually the negative z direction in OpenGL style.
  Vector4 rayClip(x, y, -1.0f, 1.0f);

  Vector4 rayView = camera.getProj().Inverse() * rayClip;
  rayView.Z = -1.0f;
  rayView.W = 0.0f;

  Vector3 rayWorld = Vector3(camera.getView().Inverse() * rayView);
  rayWorld.Normalize();

  return Ray(camera.getParentTransform().getPosition(), rayWorld);
}

Scene::Scene(const std::shared_ptr<InputHandler> &inputHandler) : _objectAddedToScene(false),
                                                                  _scenePrepDuration(0),
                                                                  _inputHandler(inputHandler)
{
}

Scene::~Scene() {}

bool Scene::init(const Vector2I &windowDims, std::shared_ptr<RenderDevice> renderDevice)
{
  _windowDims = windowDims;
  _rootGameObject.reset(new GameObject("root"));

  _renderDevice = renderDevice;
  _renderer.reset(new Renderer(windowDims));
  return _renderer->init(_renderDevice);
}

void Scene::update(float32 dt)
{
  _rootGameObject->update(dt);
}

void Scene::drawFrame()
{
  std::chrono::time_point start = std::chrono::high_resolution_clock::now();
  if (_renderDevice == nullptr || _renderer == nullptr)
  {
    std::cerr << "Renderer not initialized." << std::endl;
    return;
  }

  Ray ray = buildRayFromMouseCoords(_mouseCoordinates, _windowDims, *_mainCamera.get());
  std::vector<std::shared_ptr<Light>> lights;
  std::vector<std::shared_ptr<Drawable>> aabbDrawables, allDrawables, opaqueDrawables, transparentDrawables;
  std::vector<std::shared_ptr<GameObject>> mousePickedObjects;
  drawGameObject(_rootGameObject,
                 opaqueDrawables,
                 transparentDrawables,
                 allDrawables,
                 aabbDrawables,
                 mousePickedObjects,
                 lights,
                 ray,
                 _mainCamera);

  std::sort(opaqueDrawables.begin(), opaqueDrawables.end(), [&](const std::shared_ptr<Drawable> &a, const std::shared_ptr<Drawable> &b) -> bool
            { return _mainCamera->distanceFrom(a->getPosition()) < _mainCamera->distanceFrom(b->getPosition()); });
  std::sort(transparentDrawables.begin(), transparentDrawables.end(), [&](const std::shared_ptr<Drawable> &a, const std::shared_ptr<Drawable> &b) -> bool
            { return _mainCamera->distanceFrom(a->getPosition()) > _mainCamera->distanceFrom(b->getPosition()); });
  std::sort(mousePickedObjects.begin(), mousePickedObjects.end(), [&](const std::shared_ptr<GameObject> &a, const std::shared_ptr<GameObject> &b) -> bool
            { return _mainCamera->distanceFrom(a->getGlobalTransform().getPosition()) < _mainCamera->distanceFrom(b->getGlobalTransform().getPosition()); });

  setAabbDrawOnGameObject(SELECTED_GAME_OBJECT, false);
  SELECTED_GAME_OBJECT = mousePickedObjects.front();
  setAabbDrawOnGameObject(SELECTED_GAME_OBJECT, true);

  std::chrono::time_point end = std::chrono::high_resolution_clock::now();
  _scenePrepDuration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  _renderer->drawFrame(_renderDevice,
                       aabbDrawables,
                       opaqueDrawables,
                       transparentDrawables,
                       allDrawables,
                       lights,
                       _mainCamera);
}

void Scene::drawDebugUi()
{
  ImGui::BeginChild("SceneGraph", ImVec2(ImGui::GetContentRegionAvail().x, 300), false, ImGuiWindowFlags_HorizontalScrollbar);
  drawSceneGraphEditor(_rootGameObject);
  drawGameObjectInspector(SELECTED_GAME_OBJECT);
  ImGui::EndChild();

  _renderer->drawDebugUi();

  ImGui::Separator();
  {
    if (ImGui::CollapsingHeader("Frame Profiler"))
    {
      float32 scenePrepDuration = static_cast<float32>(_scenePrepDuration) * 1e-6f;
      ImGui::Text("Scene Prep: (%.3f ms)", scenePrepDuration);
      float32 totalDuration = scenePrepDuration;
      for (auto &timings : _renderer->getRenderPassTimings())
      {
        float32 duration = static_cast<float32>(timings.Duration) * 1e-6f;
        ImGui::Text("%s: (%.3f ms)", timings.Name.c_str(), duration);
        totalDuration += duration;
      }
      ImGui::Text("All: (%.3f ms)", totalDuration);
    }
  }
}

void Scene::drawSceneGraphEditor(const std::shared_ptr<GameObject> &gameObject)
{
  ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | (SELECTED_GAME_OBJECT == gameObject ? ImGuiTreeNodeFlags_Selected : 0);

  if (!gameObject->hasChildNodes())
  {
    flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    ImGui::TreeNodeEx(gameObject->getName().c_str(), flags);
    if (ImGui::IsItemClicked())
    {
      setAabbDrawOnGameObject(SELECTED_GAME_OBJECT, false);
      SELECTED_GAME_OBJECT = gameObject;
      setAabbDrawOnGameObject(SELECTED_GAME_OBJECT, true);
    }
    else
    {
      bool open = ImGui::TreeNodeEx(gameObject->getName().c_str(), flags);
      if (ImGui::IsItemClicked())
      {
        setAabbDrawOnGameObject(SELECTED_GAME_OBJECT, false);
        SELECTED_GAME_OBJECT = gameObject;
        setAabbDrawOnGameObject(SELECTED_GAME_OBJECT, true);
      }
      if (open)
      {
        for (const auto &childNode : gameObject->getChildNodes())
        {
          drawSceneGraphEditor(childNode);
        }
        ImGui::TreePop();
      }
    }
  }
}

void Scene::drawGameObjectInspector(const std::shared_ptr<GameObject> &gameObject)
{
  if (gameObject == nullptr)
  {
    return;
  }

  ImGui::Begin("Inspector", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize);
  gameObject->drawInspector();

  ImVec2 screenSize = ImGui::GetIO().DisplaySize;
  auto windowPos = ImVec2(screenSize.x - ImGui::GetWindowWidth(), 0);
  ImGui::SetWindowPos(windowPos);
  ImGui::End();
}

void Scene::setAabbDrawOnGameObject(const std::shared_ptr<GameObject> &gameObject, bool enableAabbDraw)
{
  if (gameObject == nullptr)
  {
    return;
  }

  if (gameObject->hasComponent<Drawable>())
  {
    gameObject->getComponent<Drawable>()->enableDrawAabb(enableAabbDraw);
  }
}

void Scene::drawGameObject(const std::shared_ptr<GameObject> &gameObject,
                           std::vector<std::shared_ptr<Drawable>> &opaqueDrawables,
                           std::vector<std::shared_ptr<Drawable>> &transparentDrawables,
                           std::vector<std::shared_ptr<Drawable>> &allDrawables,
                           std::vector<std::shared_ptr<Drawable>> &aabbDrawables,
                           std::vector<std::shared_ptr<GameObject>> &mousePickedObjects,
                           std::vector<std::shared_ptr<Light>> &lights,
                           const Ray &mouseCoordsProjectedRay,
                           const std::shared_ptr<Camera> &camera) const
{
  if (gameObject->hasComponent<Drawable>())
  {
    std::shared_ptr<Drawable> drawable(gameObject->getComponent<Drawable>());

    // Perform camera frustrum culling.
    if (camera->contains(drawable->getAabb(), Transform(drawable->getMatrix())))
    {
      if (drawable->getMaterial()->hasOpacityTexture())
      {
        transparentDrawables.push_back(drawable);
      }
      else
      {
        opaqueDrawables.push_back(drawable);
      }
    }

    // Perform mouse picking.
    Vector3 extents(drawable->getAabb().getExtents());
    if (mouseCoordsProjectedRay.Intersects(Aabb(drawable->getPosition(), extents.X, extents.Y, extents.Z)) &&
        _inputHandler->isButtonPressed(Button::Button_LMouse))
    {
      mousePickedObjects.push_back(gameObject);
    }

    if (drawable->shouldDrawAabb())
    {
      aabbDrawables.push_back(drawable);
    }

    allDrawables.push_back(drawable);
  }

  if (gameObject->hasComponent<Light>())
  {
    std::shared_ptr<Light> light(gameObject->getComponent<Light>());
    lights.push_back(light);
  }

  if (gameObject->hasChildNodes())
  {
    for (const auto &childNode : gameObject->getChildNodes())
    {
      drawGameObject(childNode,
                     opaqueDrawables,
                     transparentDrawables,
                     allDrawables,
                     aabbDrawables,
                     mousePickedObjects,
                     lights,
                     mouseCoordsProjectedRay,
                     camera);
    }
  }
}