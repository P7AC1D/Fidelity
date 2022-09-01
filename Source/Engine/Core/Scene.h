#pragma once
#include <memory>
#include <vector>

#include "../Rendering/Drawable.h"
#include "../Rendering/Camera.h"
#include "../Rendering/Light.h"
#include "Maths.h"
#include "Types.hpp"

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

  Camera &getCamera() { return _camera; }
  const std::vector<Drawable> &getDrawables() const { return _drawables; }

  // TODO Remove this and better abstract dependencies
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

  std::vector<Drawable> _drawables;
  std::vector<Light> _lights;
  Camera _camera;

  std::shared_ptr<DeferredRenderer> _deferredRenderer;
  std::shared_ptr<RenderDevice> _renderDevice;
};