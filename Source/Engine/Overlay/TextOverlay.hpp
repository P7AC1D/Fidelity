#pragma once
#include <memory>
#include <string>

#include "../Maths/Colour.hpp"
#include "../Maths/Vector2I.hpp"
#include "../Rendering/Texture.hpp"
#include "Font.hpp"

namespace Rendering
{
class VertexBuffer;
}

struct TextOverlayDesc
{
  Vector2i Position;
  uint32 LineWidth;
  std::string Text;
  std::shared_ptr<Font> Font;
  std::shared_ptr<Rendering::Texture> Atlas;
  Colour TextColour = Colour::Black;
  bool Centered = false;
};

class TextOverlay
{
public:
  TextOverlay(const TextOverlayDesc& desc, uint32 windowWidth, uint32 windowHeight);

  inline TextOverlayDesc GetDesc() const { return _desc; }

  void Draw();

private:
  void UploadMeshData();

private:
  bool _dirty;
  uint32 _windowWidth;
  uint32 _windowHeight;
  TextOverlayDesc _desc;
  std::unique_ptr<Rendering::VertexBuffer> _vertexBuffer;
};