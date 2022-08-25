#pragma once
#include <memory>
#include <string>
#include <vector>

#include "Types.hpp"

class Drawable;
class Camera;
class Light;

class Scene
{
public:
  std::shared_ptr<Drawable> createDrawable();
  std::shared_ptr<Camera> createCamera();
  std::shared_ptr<Light> createLight();

  void update(float64 dt);
  void draw(std::shared_ptr<Camera> camera);

private:
  std::vector<Drawable> _drawables;
  std::vector<Light> _lights;
  std::vector<Camera> _cameras;
};