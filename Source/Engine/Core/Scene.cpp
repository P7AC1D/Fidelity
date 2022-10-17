#include "Scene.h"

#include <iostream>
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
#include "SceneGraph.h"

static int64 SELECTED_GAME_OBJECT_INDEX = -1;

Scene::Scene() : _objectAddedToScene(false), _scenePrepDuration(0) {}

Scene::~Scene() {}

bool Scene::init(const Vector2I &windowDims, std::shared_ptr<RenderDevice> renderDevice)
{
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
  _gameObjects.insert(std::pair<uint64, GameObject>(index, std::move(GameObject(name, index))));
  _objectAddedToScene = true;
  return _gameObjects[index++];
}

void Scene::addChildToNode(GameObject &parent, GameObject &child)
{
  _sceneGraph->addChildToNode(parent.getIndex(), child.getIndex());
  parent.addChildNode(child);
}

void Scene::update(float64 dt)
{
  for (auto &gameObject : _gameObjects)
  {
    gameObject.second.update(dt);
  }

  for (auto componentType : _components)
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
  std::vector<std::shared_ptr<Drawable>> aabbDrawables, allDrawables, opaqueDrawables, transparentDrawables;
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

void Scene::drawSceneGraphUi(int64 nodeIndex)
{
  ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | (SELECTED_GAME_OBJECT_INDEX == nodeIndex ? ImGuiTreeNodeFlags_Selected : 0);

  GameObject &gameObject = _gameObjects[nodeIndex];

  if (!_sceneGraph->doesNodeHaveChildren(nodeIndex))
  {
    flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    ImGui::TreeNodeEx(gameObject.getName().c_str(), flags);
    if (ImGui::IsItemClicked())
    {
      setAabbDrawOnGameObject(SELECTED_GAME_OBJECT_INDEX, false);
      SELECTED_GAME_OBJECT_INDEX = nodeIndex;
      setAabbDrawOnGameObject(SELECTED_GAME_OBJECT_INDEX, true);
    }
  }
  else
  {
    bool open = ImGui::TreeNodeEx(gameObject.getName().c_str(), flags);
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

  GameObject &gameObject = _gameObjects[selectedGameObjectIndex];
  gameObject.drawInspector();

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

  GameObject &gameObject = _gameObjects[gameObjectIndex];
  if (gameObject.hasComponent<Drawable>())
  {
    gameObject.getComponent<Drawable>().enableDrawAabb(enableAabbDraw);
  }
}