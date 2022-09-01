#include "Scene.h"

#include <iostream>
#include <set>
#include <string>
#include <vector>

#include "../Utility/ModelLoader.hpp"
#include "../Rendering/DeferredRenderer.h"
#include "../RenderApi/RenderDevice.hpp"

bool Scene::init(const Vector2I &windowDims, std::shared_ptr<RenderDevice> renderDevice)
{
  _renderDevice = renderDevice;
  _deferredRenderer.reset(new DeferredRenderer(windowDims));
  return _deferredRenderer->init(_renderDevice);
}

Drawable &Scene::createDrawable()
{
  _drawables.push_back(Drawable(std::string()));
  return _drawables[_drawables.size() - 1];
}

Light &Scene::createLight()
{
  _lights.push_back(Light());

  return _lights[_lights.size() - 1];
}

void Scene::update(float64 dt)
{
  _camera.update(dt);

  for (auto &drawable : _drawables)
  {
    drawable.update(dt);
  }

  for (auto &light : _lights)
  {
    light.update(dt);
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

  std::vector<uint64> aabbDrawableIndices;
  for (uint64 i = 0; i < _drawables.size(); i++)
  {
    auto &drawable = _drawables[i];
    if (_camera.intersectsFrustrum(drawable))
    {
      culledDrawables.insert(DrawableSortMap(_camera.distanceFrom(drawable), i));
      if (drawable.shouldDrawAabb())
      {
        aabbDrawableIndices.push_back(i);
      }
    }
  }

  std::vector<uint64> culledDrawableIndices;
  culledDrawableIndices.reserve(culledDrawables.size());
  for (auto &culledDrawable : culledDrawables)
  {
    culledDrawableIndices.push_back(culledDrawable.Index);
  }

  _deferredRenderer->drawFrame(_renderDevice, culledDrawableIndices, aabbDrawableIndices, _drawables, _lights, _camera);
}