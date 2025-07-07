#include "DrawableComponent.h"

#include "../Core/TransformComponent.h"
#include "../Core/GameObject.h"
#include "../UI/ImGui/imgui.h"
#include "../UI/UiManager.hpp"
#include "StaticMesh.h"
#include "Material.h"

DrawableComponent::DrawableComponent()
{
}

DrawableComponent::DrawableComponent(std::shared_ptr<StaticMesh> mesh, std::shared_ptr<Material> material)
    : _mesh(mesh), _material(material)
{
  if (_mesh)
  {
    onMeshChanged();
  }
}

void DrawableComponent::initialize()
{
  // Initialize bounds if we have a mesh
  if (_mesh)
  {
    onMeshChanged();
  }
}

void DrawableComponent::activate()
{
  // Component is now active - could register with rendering system here
}

void DrawableComponent::deactivate()
{
  // Component is now inactive - could unregister from rendering system here
}

std::vector<ComponentTypeId> DrawableComponent::getDependencies() const
{
  return {getComponentTypeId<TransformComponent>()};
}

void DrawableComponent::onDependenciesResolved(GameObject &gameObject)
{
  // Get the TransformComponent from the GameObject using shared_ptr
  if (auto transformShared = gameObject.getComponentShared<TransformComponent>())
  {
    // Convert to weak_ptr for the drawable
    std::weak_ptr<TransformComponent> weakPtr = transformShared;
    setTransformComponent(weakPtr);
  }
}

void DrawableComponent::setTransformComponent(std::weak_ptr<TransformComponent> transform)
{
  _transformComponent = transform;
  markDirty();
}

ComponentTypeId DrawableComponent::getTypeId() const
{
  return GetTypeId();
}

void DrawableComponent::drawInspector()
{
  if (ImGui::CollapsingHeader("Drawable"))
  {
    // Visibility controls
    drawRenderingFlagsInspector();

    ImGui::Separator();

    // Material controls
    if (_material)
    {
      drawMaterialInspector();
    }
    else
    {
      ImGui::Text("No Material");
    }

    ImGui::Separator();

    // Mesh info
    if (_mesh)
    {
      ImGui::Text("Mesh: %s", "StaticMesh"); // TODO: Get mesh name
      ImGui::Text("Vertices: %d", 0);        // TODO: Get vertex count from mesh
      ImGui::Text("Triangles: %d", 0);       // TODO: Get triangle count from mesh
    }
    else
    {
      ImGui::Text("No Mesh");
    }

    // Bounds info
    if (_mesh)
    {
      const Aabb &bounds = getLocalBounds();
      Vector3 min = bounds.getMin();
      Vector3 max = bounds.getMax();
      Vector3 size = max - min;

      ImGui::Text("Local Bounds:");
      ImGui::Text("  Min: (%.2f, %.2f, %.2f)", min.X, min.Y, min.Z);
      ImGui::Text("  Max: (%.2f, %.2f, %.2f)", max.X, max.Y, max.Z);
      ImGui::Text("  Size: (%.2f, %.2f, %.2f)", size.X, size.Y, size.Z);
    }
  }
}

void DrawableComponent::setMesh(std::shared_ptr<StaticMesh> mesh)
{
  _mesh = mesh;
  onMeshChanged();
}

void DrawableComponent::setMaterial(std::shared_ptr<Material> material)
{
  _material = material;
}

const Aabb &DrawableComponent::getWorldBounds() const
{
  if (!_boundsValid)
  {
    updateWorldBounds();
  }
  return _worldBounds;
}

const TransformComponent *DrawableComponent::getCachedTransform() const
{
  if (auto transform = _transformComponent.lock())
  {
    return transform.get();
  }
  return nullptr;
}

const Matrix4 &DrawableComponent::getWorldMatrix() const
{
  if (const TransformComponent *transform = getCachedTransform())
  {
    return transform->getWorldMatrix();
  }
  static Matrix4 identity = Matrix4::Identity;
  return identity;
}

Vector3 DrawableComponent::getWorldPosition() const
{
  if (const TransformComponent *transform = getCachedTransform())
  {
    return transform->getPosition();
  }
  return Vector3::Zero;
}

void DrawableComponent::updateWorldBounds() const
{
  if (!_mesh)
  {
    _worldBounds = Aabb();
    _boundsValid = true;
    return;
  }

  const TransformComponent *transform = getCachedTransform();
  if (transform)
  {
    // Transform local bounds to world space
    const Matrix4 &worldMatrix = transform->getWorldMatrix();

    // Get the 8 corners of the local AABB
    Vector3 min = _localBounds.getMin();
    Vector3 max = _localBounds.getMax();

    Vector3 corners[8] = {
        Vector3(min.X, min.Y, min.Z),
        Vector3(max.X, min.Y, min.Z),
        Vector3(min.X, max.Y, min.Z),
        Vector3(max.X, max.Y, min.Z),
        Vector3(min.X, min.Y, max.Z),
        Vector3(max.X, min.Y, max.Z),
        Vector3(min.X, max.Y, max.Z),
        Vector3(max.X, max.Y, max.Z)};

    // Transform all corners and find new min/max
    Vector3 worldMin = worldMatrix * corners[0];
    Vector3 worldMax = worldMin;

    for (int i = 1; i < 8; i++)
    {
      Vector3 worldCorner = worldMatrix * corners[i];

      worldMin.X = std::min(worldMin.X, worldCorner.X);
      worldMin.Y = std::min(worldMin.Y, worldCorner.Y);
      worldMin.Z = std::min(worldMin.Z, worldCorner.Z);

      worldMax.X = std::max(worldMax.X, worldCorner.X);
      worldMax.Y = std::max(worldMax.Y, worldCorner.Y);
      worldMax.Z = std::max(worldMax.Z, worldCorner.Z);
    }

    _worldBounds = Aabb(worldMax, worldMin);
  }
  else
  {
    // No transform available, world bounds = local bounds
    _worldBounds = _localBounds;
  }

  _boundsValid = true;
}

void DrawableComponent::onMeshChanged()
{
  if (_mesh)
  {
    _localBounds = _mesh->getAabb();
  }
  else
  {
    _localBounds = Aabb();
  }
  markDirty();
}

void DrawableComponent::drawMaterialInspector()
{
  if (!_material)
    return;

  ImGui::Text("Material Properties");

  // Diffuse color
  Colour diffuse = _material->getDiffuseColour();
  float32 rawCol[3] = {diffuse[0] / 255.0f, diffuse[1] / 255.0f, diffuse[2] / 255.0f};
  if (ImGui::ColorEdit3("Diffuse", rawCol))
  {
    _material->setDiffuseColour(Colour(rawCol[0] * 255, rawCol[1] * 255, rawCol[2] * 255));
  }

  // PBR properties
  if (_material->hasDiffuseTexture())
  {
    bool diffuseEnabled = _material->diffuseTextureEnabled();
    if (ImGui::Checkbox("Diffuse Texture", &diffuseEnabled))
    {
      _material->enableDiffuseTexture(diffuseEnabled);
    }
  }

  if (_material->hasNormalTexture())
  {
    bool normalEnabled = _material->normalTextureEnabled();
    if (ImGui::Checkbox("Normal Texture", &normalEnabled))
    {
      _material->enableNormalTexture(normalEnabled);
    }
  }

  if (_material->hasMetallicTexture())
  {
    bool metallicEnabled = _material->metallicTextureEnabled();
    if (ImGui::Checkbox("Metallic Texture", &metallicEnabled))
    {
      _material->enableMetallicTexture(metallicEnabled);
    }
  }

  if (_material->hasRoughnessTexture())
  {
    bool roughnessEnabled = _material->roughnessTextureEnabled();
    if (ImGui::Checkbox("Roughness Texture", &roughnessEnabled))
    {
      _material->enableRoughnessTexture(roughnessEnabled);
    }
  }

  if (_material->hasOcclusionTexture())
  {
    bool occlusionEnabled = _material->occlusionTextureEnabled();
    if (ImGui::Checkbox("Occlusion Texture", &occlusionEnabled))
    {
      _material->enableOcclusionTexture(occlusionEnabled);
    }
  }

  if (_material->hasOpacityTexture())
  {
    bool opacityEnabled = _material->opacityTextureEnabled();
    if (ImGui::Checkbox("Opacity Texture", &opacityEnabled))
    {
      _material->enableOppacityTexture(opacityEnabled);
    }
  }

  // Material sliders
  if (!_material->hasMetallicTexture() || !_material->metallicTextureEnabled())
  {
    float32 metalness = _material->getMetalness();
    if (ImGui::SliderFloat("Metallic", &metalness, 0.0f, 1.0f))
    {
      _material->setMetalness(metalness);
    }
  }

  if (!_material->hasRoughnessTexture() || !_material->roughnessTextureEnabled())
  {
    float32 roughness = _material->getRoughness();
    if (ImGui::SliderFloat("Roughness", &roughness, 0.0f, 1.0f))
    {
      _material->setRoughness(roughness);
    }
  }

  // Texture preview
  static const char *textureNames[] = {"Albedo", "Normal", "Metallic", "Roughness", "Opacity"};
  static int currentTexture = 0;

  if (ImGui::Combo("Preview Texture", &currentTexture, textureNames, 5))
  {
    // Texture selection changed
  }

  drawTexturePreview(textureNames[currentTexture], currentTexture);
}

void DrawableComponent::drawRenderingFlagsInspector()
{
  ImGui::Text("Rendering Properties");

  if (ImGui::Checkbox("Visible", &_visible))
  {
    // Visibility changed
  }

  if (ImGui::Checkbox("Cast Shadows", &_castShadows))
  {
    // Shadow casting changed
  }

  if (ImGui::Checkbox("Receive Shadows", &_receiveShadows))
  {
    // Shadow receiving changed
  }

  if (ImGui::Checkbox("Draw AABB", &_drawAabb))
  {
    // AABB drawing changed
  }
}

void DrawableComponent::drawTexturePreview(const std::string &textureName, int textureIndex)
{
  if (!_material)
    return;

  std::shared_ptr<class Texture> texture;

  switch (textureIndex)
  {
  case 0: // Albedo
    texture = _material->getDiffuseTexture();
    break;
  case 1: // Normal
    texture = _material->getNormalTexture();
    break;
  case 2: // Metallic
    texture = _material->getMetallicTexture();
    break;
  case 3: // Roughness
    texture = _material->getRoughnessTexture();
    break;
  case 4: // Opacity
    texture = _material->getOpacityTexture();
    break;
  }

  if (texture)
  {
    UiManager::addTexture(reinterpret_cast<uint64>(&texture), texture);
    ImGui::Image(
        &texture,
        ImVec2(200, 200),
        ImVec2(0.0f, 0.0f),
        ImVec2(1.0f, 1.0f),
        ImVec4(1.0f, 1.0f, 1.0f, 1.0f),
        ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
  }
  else
  {
    ImGui::Text("No %s texture", textureName.c_str());
  }
}
