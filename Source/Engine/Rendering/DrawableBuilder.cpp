#include "DrawableBuilder.hpp"

#include "Material.h"
#include "StaticMesh.h"

DrawableBuilder &DrawableBuilder::withMesh(std::shared_ptr<StaticMesh> mesh)
{
  _mesh = mesh;
  return *this;
}

DrawableBuilder &DrawableBuilder::withMaterial(std::shared_ptr<Material> material)
{
  _material = material;
  return *this;
}

std::shared_ptr<Component> DrawableBuilder::build()
{
  std::shared_ptr<Drawable> drawable(new Drawable());
  drawable->setMaterial(_material);
  drawable->setMesh(_mesh);
  return drawable;
}