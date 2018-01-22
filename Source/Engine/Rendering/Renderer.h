#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "../Core/Types.hpp"
#include "../Maths/Vector3.hpp"
#include "../SceneManagement/Light.h"
#include "../SceneManagement/OrbitalCamera.h"
#include "../SceneManagement/Transform.h"
#include "Renderable.hpp"

namespace UI
{
class Panel;
class UiManager;
}

namespace Rendering
{
class ConstantBuffer;
class CubeMap;
class Shader;
class ShaderCollection;
class StaticMesh;
class VertexBuffer;
enum class RenderingTechnique;

enum class ClearType
{
  Color,
  Depth,
  All,
};

struct RenderableItem
{
  RenderableItem(std::shared_ptr<Renderable> renderable, std::shared_ptr<Transform> transform):
    Renderable(renderable),
    Transform(transform)
  {}

  std::shared_ptr<Renderable> Renderable;
  std::shared_ptr<Transform> Transform;
};

class Renderer
{
public:
  Renderer(int32 renderWidth, int32 renderHeight);
  virtual ~Renderer();

  inline void SetAmbientLight(const Vector3& ambientLight) { _ambientLight = ambientLight; }
  inline void PushRenderable(std::shared_ptr<Renderable> renderable, std::shared_ptr<Transform> transform) 
  { 
    _renderables.emplace_back(renderable, transform);
  }
  inline void PushPointLight(const Light& pointLight) { _pointLights.push_back(pointLight); }
  inline void PushDirectionalLight(const Light& directionalLight) { _directionalLights.push_back(directionalLight); }

  void SetViewport(int32 renderWidth, int32 renderHeight);
  void SetClearColour(const Vector3& colour);

  void DrawScene(OrbitalCamera& camera);

  void DrawUI(std::vector<std::shared_ptr<UI::Panel>> panelCollection);
  bool Initialize();

  static void SetVertexAttribPointers(StaticMesh* staticMesh, int32 stride);

private:
  void UploadCameraData(OrbitalCamera& camera);
  void UploadPointLightData(const Light& pointLight);
  void UploadDirectionalLightData(const Light& directionalLight);

  void DirectionalLightRender(OrbitalCamera& camera);
  void PointLightRender(OrbitalCamera& camera);

  void ClearBuffer(ClearType clearType);

private:
  std::vector<RenderableItem> _renderables;
  std::vector<Light> _pointLights;
  std::vector<Light> _directionalLights;
  
  std::unique_ptr<ShaderCollection> _shaderCollection;

  std::unique_ptr<ConstantBuffer> _cameraBuffer;
  std::unique_ptr<ConstantBuffer> _lightBuffer;
  std::unique_ptr<ConstantBuffer> _ambientLightBuffer;
  bool _projectionMatrixDirty;
  bool _viewMatrixDirty;

  std::unique_ptr<VertexBuffer> _guiQuadVertexData;
  std::unique_ptr<VertexBuffer> _skyBoxVertexData;

  int32 _renderWidth;
  int32 _renderHeight;
  Vector3 _ambientLight;
};
}