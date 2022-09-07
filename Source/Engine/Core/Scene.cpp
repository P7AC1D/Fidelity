#include "Scene.h"

#include <iostream>
#include <set>
#include <string>
#include <vector>

#include "../UI/ImGui/imgui.h"
#include "../Utility/ModelLoader.hpp"
#include "../Rendering/DeferredRenderer.h"
#include "../Rendering/Drawable.h"
#include "../Rendering/Light.h"
#include "../RenderApi/RenderDevice.hpp"
#include "GameObject.h"

bool Scene::init(const Vector2I &windowDims, std::shared_ptr<RenderDevice> renderDevice)
{
  createGameObject("root");

  _renderDevice = renderDevice;
  _deferredRenderer.reset(new DeferredRenderer(windowDims));
  return _deferredRenderer->init(_renderDevice);
}

GameObject &Scene::createGameObject(const std::string &name)
{
  _sceneGraph.push_back(Hierarchy{ _sceneGraph.size(), -1, -1});
  auto gameObject = std::shared_ptr<GameObject>(new GameObject(name, _sceneGraph.size() - 1));
  _gameObjects.push_back(gameObject);
  return *(_gameObjects.back().get());
}

void Scene::addChildToNode(GameObject &parent, GameObject &child)
{
  int64 index = _sceneGraph[parent.getIndex()].Child;
  if (index == -1)
  {
    _sceneGraph[parent.getIndex()].Child = child.getIndex();
    return;
  }

  while (_sceneGraph[index].Sibling != -1)
  {
    index = _sceneGraph[index].Sibling;
  }

  _sceneGraph[index].Sibling = child.getIndex();
}

void Scene::update(float64 dt)
{
  _camera.update(dt);

  for (auto componentType : _components)
  {
    for (auto &component : _components[componentType.first])
    {
      component->update(dt);
    }
  }

  for (auto &gameObject : _gameObjects)
  {
    gameObject->update(dt);
  }
}

void Scene::drawFrame() const
{
  if (_renderDevice == nullptr || _deferredRenderer == nullptr)
  {
    std::cerr << "Renderer not initialized." << std::endl;
    return;
  }

  auto compare = [](DrawableSortMap a, DrawableSortMap b)
  { return a.DistanceToCamera < b.DistanceToCamera; };
  std::multiset<DrawableSortMap, decltype(compare)> culledDrawables(compare);

  auto findIter = _components.find(ComponentType::Drawable);
  if (findIter == _components.end())
  {
    return;
  }

  std::vector<std::shared_ptr<Drawable>> aabbDrawables;
  for (auto component : findIter->second)
  {
    auto drawable = std::dynamic_pointer_cast<Drawable>(component);
    if (_camera.intersectsFrustrum(drawable->getAabb()))
    {
      culledDrawables.insert(DrawableSortMap(_camera.distanceFrom(drawable->getPosition()), drawable));
      if (drawable->shouldDrawAabb())
      {
        aabbDrawables.push_back(drawable);
      }
    }
  }

  std::vector<std::shared_ptr<Light>> lights;
  for (auto component : _components.find(ComponentType::Light)->second)
  {
    auto light = std::dynamic_pointer_cast<Light>(component);
    lights.push_back(light);
  }

  std::vector<std::shared_ptr<Drawable>> drawables;
  for (auto culledDrawable : _components.find(ComponentType::Drawable)->second)
  {
    drawables.push_back(std::dynamic_pointer_cast<Drawable>(culledDrawable));
  }

  _deferredRenderer->drawFrame(_renderDevice, aabbDrawables, drawables, lights, _camera);
}

void Scene::updateInspector()
{
  ImGui::BeginChild("SceneGraph", ImVec2(ImGui::GetContentRegionAvail().x, 200), false, ImGuiWindowFlags_HorizontalScrollbar);

  ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick /* | (selectedActor == parentNode ? ImGuiTreeNodeFlags_Selected : 0)*/;
  ImGui::Separator();
  if (ImGui::TreeNode("Scene"))
  {
    

    Hierarchy& node(_sceneGraph.front());

    auto gameObject = _gameObjects.front();
    bool open = ImGui::TreeNodeEx(gameObject->getName().c_str(), flags);
    if (open)
    {
      while (node.Child != -1)
      {
        node = _sceneGraph[node.Child];
        ImGui::TreeNodeEx(gameObject->getName().c_str(), flags);

        ImGui::TreePop();
      }
    }

    
    ImGui::TreePop();
  }

  ImGui::EndChild();
}