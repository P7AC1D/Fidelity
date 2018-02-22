#pragma once
#include <string>
#include <vector>
#include "../Maths/Colour.hpp"
#include "GuiElement.hpp"

namespace Rendering
{
class VertexBuffer;
}

struct GuiPanelDesc
{
  std::string Name;
  uint32 Width = 100;
  uint32 Height = 100;
  Vector2i Position = Vector2i::Zero;
  Colour Colour = Colour::Black;
};

class GuiPanel : GuiElement
{
public:
  GuiPanel(const GuiPanelDesc& desc);
  ~GuiPanel() {}

  void SetPosition(const Vector2i& position);
  void SetDimensions(const Vector2i& dimensions);

  inline Vector2i GetPosition() const { return _position; }
  inline Vector2i GetDimensions() const { return _dimensions; }
  inline Colour GetColour() const { return _colour; }

  void Draw() override;

private:
  void UploadToGpu();

private:
  std::string _name;
  std::vector<std::shared_ptr<GuiElement>> _childElements;
  std::shared_ptr<Rendering::VertexBuffer> _vertexBuffer;  
  Colour _colour;
  bool _dirty;
};
