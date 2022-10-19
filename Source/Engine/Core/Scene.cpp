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
#include "SceneGraph.h"

static int64 SELECTED_GAME_OBJECT_INDEX = -1;

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
  _sceneGraph.reset(new SceneGraph());
  createGameObject("root");

  _renderDevice = renderDevice;
  _renderer.reset(new Renderer(windowDims));
  return _renderer->init(_renderDevice);
}

GameObject &Scene::createGameObject(const std::string &name)
{
  static uint64 index = 0;

  _sceneGraph->addNode(index);
  _gameObjects.insert(std::pair<uint64, std::shared_ptr<GameObject>>(index, new GameObject(name, index)));
  _objectAddedToScene = true;
  return *_gameObjects[index++].get();
}

void Scene::addChildToNode(GameObject &parent, GameObject &child)
{
  _sceneGraph->addChildToNode(parent.getIndex(), child.getIndex());
  parent.addChildNode(child);
}

void Scene::update(float32 dt)
{
  for (const auto &gameObject : _gameObjects)
  {
    gameObject.second->update(dt);
  }

  for (const auto &componentType : _components)
  {
    for (auto &component : _components[componentType.first])
    {
      component->update(dt);
    }
  }
}

void Scene::drawFrame()
{
  std::chrono::time_point start = std::chrono::high_resolution_clock::now();
  if (_renderDevice == nullptr || _renderer == nullptr)
  {
    std::cerr << "Renderer not initialized." << std::endl;
    return;
  }

  auto drawableFindIter = _components.find(ComponentType::Drawable);
  if (drawableFindIter == _components.end())
  {
    return;
  }

  auto cameraFindIter = _components.find(ComponentType::Camera);
  if (cameraFindIter == _components.end())
  {
    return;
  }

  std::shared_ptr<Camera> camera(std::static_pointer_cast<Camera>(cameraFindIter->second[0]));
  performObjectPicker(*camera.get());

  std::vector<std::shared_ptr<Drawable>>
      aabbDrawables, allDrawables, opaqueDrawables, transparentDrawables, mousePickedDrawables;
  for (auto component : drawableFindIter->second)
  {
    auto drawable = std::dynamic_pointer_cast<Drawable>(component);
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

    if (drawable->shouldDrawAabb())
    {
      aabbDrawables.push_back(drawable);
    }

    allDrawables.push_back(drawable);
  }

  std::sort(opaqueDrawables.begin(), opaqueDrawables.end(), [&](const std::shared_ptr<Drawable> &a, const std::shared_ptr<Drawable> &b) -> bool
            { return camera->distanceFrom(a->getPosition()) < camera->distanceFrom(b->getPosition()); });

  std::vector<std::shared_ptr<Light>> lights;
  for (auto component : _components.find(ComponentType::Light)->second)
  {
    auto light = std::dynamic_pointer_cast<Light>(component);
    lights.push_back(light);
  }

  std::chrono::time_point end = std::chrono::high_resolution_clock::now();
  _scenePrepDuration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  _renderer->drawFrame(_renderDevice,
                       aabbDrawables,
                       opaqueDrawables,
                       transparentDrawables,
                       allDrawables,
                       lights,
                       camera);
}

void Scene::drawDebugUi()
{
  ImGui::BeginChild("SceneGraph", ImVec2(ImGui::GetContentRegionAvail().x, 300), false, ImGuiWindowFlags_HorizontalScrollbar);
  drawSceneGraphUi(0);
  drawGameObjectInspector(SELECTED_GAME_OBJECT_INDEX);
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

void Scene::performObjectPicker(const Camera &camera)
{
  Ray ray = buildRayFromMouseCoords(_mouseCoordinates, _windowDims, camera);

  std::vector<std::shared_ptr<GameObject>> rayCastedObjects;
  for (const auto &gameObject : _gameObjects)
  {
    if (gameObject.second->hasComponent<Drawable>())
    {
      Drawable &drawable = gameObject.second->getComponent<Drawable>();

      Vector3 extents(drawable.getAabb().getExtents());
      if (ray.Intersects(Aabb(drawable.getPosition(), extents.X, extents.Y, extents.Z)) &&
          _inputHandler->isButtonPressed(Button::Button_LMouse))
      {
        rayCastedObjects.push_back(gameObject.second);
      }
    }
  }

  if (rayCastedObjects.empty())
  {
    return;
  }

  std::sort(rayCastedObjects.begin(), rayCastedObjects.end(), [&](const std::shared_ptr<GameObject> &a, const std::shared_ptr<GameObject> &b) -> bool
            { return camera.distanceFrom(a->getGlobalTransform().getPosition()) < camera.distanceFrom(b->getGlobalTransform().getPosition()); });

  setAabbDrawOnGameObject(SELECTED_GAME_OBJECT_INDEX, false);
  SELECTED_GAME_OBJECT_INDEX = rayCastedObjects.front()->getIndex();
  setAabbDrawOnGameObject(SELECTED_GAME_OBJECT_INDEX, true);
}

void Scene::drawSceneGraphUi(int64 nodeIndex)
{
  ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | (SELECTED_GAME_OBJECT_INDEX == nodeIndex ? ImGuiTreeNodeFlags_Selected : 0);

  const auto &gameObject = _gameObjects[nodeIndex];

  if (!_sceneGraph->doesNodeHaveChildren(nodeIndex))
  {
    flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    ImGui::TreeNodeEx(gameObject->getName().c_str(), flags);
    if (ImGui::IsItemClicked())
    {
      setAabbDrawOnGameObject(SELECTED_GAME_OBJECT_INDEX, false);
      SELECTED_GAME_OBJECT_INDEX = nodeIndex;
      setAabbDrawOnGameObject(SELECTED_GAME_OBJECT_INDEX, true);
    }
  }
  else
  {
    bool open = ImGui::TreeNodeEx(gameObject->getName().c_str(), flags);
    if (ImGui::IsItemClicked())
    {
      setAabbDrawOnGameObject(SELECTED_GAME_OBJECT_INDEX, false);
      SELECTED_GAME_OBJECT_INDEX = nodeIndex;
      setAabbDrawOnGameObject(SELECTED_GAME_OBJECT_INDEX, true);
    }
    if (open)
    {
      for (const auto &childNodeIndex : _sceneGraph->getNodeChildren(nodeIndex))
      {
        drawSceneGraphUi(childNodeIndex);
      }
      ImGui::TreePop();
    }
  }
}

void Scene::drawGameObjectInspector(int64 selectedGameObjectIndex)
{
  if (selectedGameObjectIndex == -1)
  {
    return;
  }

  ImGui::Begin("Inspector", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize);

  const auto &gameObject = _gameObjects[selectedGameObjectIndex];
  gameObject->drawInspector();

  ImVec2 screenSize = ImGui::GetIO().DisplaySize;
  auto windowPos = ImVec2(screenSize.x - ImGui::GetWindowWidth(), 0);
  ImGui::SetWindowPos(windowPos);
  ImGui::End();
}

void Scene::setAabbDrawOnGameObject(int64 gameObjectIndex, bool enableAabbDraw)
{
  if (gameObjectIndex == -1)
  {
    return;
  }

  const auto &gameObject = _gameObjects[gameObjectIndex];
  if (gameObject->hasComponent<Drawable>())
  {
    gameObject->getComponent<Drawable>().enableDrawAabb(enableAabbDraw);
  }
}