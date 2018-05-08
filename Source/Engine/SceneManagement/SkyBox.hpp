#pragma once
#include <memory>
#include <string>
#include <vector>

namespace Rendering
{
class TextureCube;
class VertexBuffer;
}

class SkyBox
{
public:
  SkyBox();

  inline void SetTexture(std::shared_ptr<Rendering::TextureCube> textureCube) { _textureCube = textureCube; }
  inline std::shared_ptr<Rendering::TextureCube> GetTexture() const { return _textureCube; }
  
  void Draw();

private:
  std::shared_ptr<Rendering::TextureCube> _textureCube;
  std::unique_ptr<Rendering::VertexBuffer> _vertexBuffer;
};