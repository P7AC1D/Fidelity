#pragma once
#include "../Maths/Vector2I.hpp"
#include "GuiElement.hpp"

namespace Rendering
{
class Texture;
class VertexBuffer;
}

struct GuiCheckBoxDesc
{
  std::string Name;
  uint32 Size = 100;
  Colour Colour = Colour::Black;
  Vector2I Position = Vector2I::Zero;
};

class GuiCheckBox : public GuiElement
{
public:
  GuiCheckBox(const GuiCheckBoxDesc& desc);
  ~GuiCheckBox() {}

  inline void SetOnToggle(const std::function<void(bool)>& onCheck) { _onToggle = onCheck; }
  inline void SetCheckedTexture(std::shared_ptr<Rendering::Texture> texture) { _checkedTexture = texture; }
  inline void SetUncheckedTexture(std::shared_ptr<Rendering::Texture> texture) { _uncheckedTexture = texture; }
  inline bool IsChecked() const { return _checked; }

  std::shared_ptr<Rendering::Shader> GetShader() const override;

private:
  void Draw() override final;

  void OnMouseClicked(Button button) override final;

private:
  std::function<void(bool)> _onToggle;
  std::shared_ptr<Rendering::Texture> _uncheckedTexture;
  std::shared_ptr<Rendering::Texture> _checkedTexture;
  std::shared_ptr<Rendering::VertexBuffer> _vertexBuffer;
  bool _checked;
};