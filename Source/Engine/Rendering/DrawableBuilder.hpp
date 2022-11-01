#pragma once

#include "../Core/Component.h"
#include "../Core/ComponentBuilder.hpp"
#include "Drawable.h"

class Material;
class StaticMesh;

class DrawableBuilder : public ComponentBuilder
{
public:
  DrawableBuilder& withMesh(std::shared_ptr<StaticMesh> mesh);
  DrawableBuilder& withMaterial(std::shared_ptr<Material> material);

  std::shared_ptr<Component> build() override;

private:
  std::shared_ptr<Material> _material;
  std::shared_ptr<StaticMesh> _mesh;
};