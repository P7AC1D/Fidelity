#pragma once
#include <vector>

#include "../Core/Types.hpp"

namespace Utility
{
class AssetManager;
}

namespace Rendering
{
enum class CubeMapOrientation
{
  Right,
  Left,
  Top,
  Bottom,
  Back,
  Front
};

class CubeMap
{
public:
  CubeMap();  
  ~CubeMap();

private:
  uint32 _id;

  friend class Utility::AssetManager;
  friend class Renderer;
};
}