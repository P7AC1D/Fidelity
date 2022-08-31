#pragma once
#include <memory>
#include <string>
#include <vector>

#include "../Rendering/DeferredRenderer.h"
#include "Camera.h"
#include "Maths.h"
#include "Types.hpp"

class Drawable;
class Light;
class DeferredRenderer;
class RenderDevice;

class Scene
{
public:
  bool init(const Vector2I &windowDims, std::shared_ptr<RenderDevice> renderDevice);

  Drawable &createDrawable();
  Light &createLight();

  void update(float64 dt);
  void drawFrame() const;

  void setDebugDisplayType(DebugDisplayType debugDisplayType);

  Camera &getCamera() { return _camera; }
  const std::vector<Drawable> &getDrawables() const { return _drawables; }

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

  std::vector<Drawable> _drawables;
  std::vector<Light> _lights;
  Camera _camera;

  std::unique_ptr<DeferredRenderer> _deferredRenderer;
  std::shared_ptr<RenderDevice> _renderDevice;
};